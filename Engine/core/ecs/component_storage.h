/*
* 组件不复制；单个组件返回引用，批量查询返回只读视图或引用集合
*/

#pragma once

#include "entity.h"
#include <cstdint>

#include <functional>
#include<GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace neon::core::ecs
{
	// 1. 基类必须拥有虚析构函数
	class ComponentStorageBase
	{
		size_t typeId;
	public:
		inline void setTypeId(size_t id) { typeId = id; }
		inline size_t getTypeId() const { return typeId; }
		virtual ~ComponentStorageBase() = default;
	};

	/// @brief 组件存储及其访问规则：单个组件通过引用访问；集合查询返回引用或非拥有视图，组件对象不会因查询而复制。
	/// 引用和视图都依赖当前存储；添加、删除或移动存储后不要继续使用先前取得的引用或视图。
	template<typename T>
	class ComponentStorage : public ComponentStorageBase
	{
		//static_assert(std::is_trivially_copyable_v<T>, "Component type must be trivially copyable");

		std::vector<Entity> entities;
		std::vector<T> components;
		std::unordered_map<Entity, std::size_t> entityToIndex;
		// 引入反向映射：通过数组索引快速找到对应的 Entity，用于 Swap-and-Pop 优化
		std::vector<Entity> indexToEntity;

		template<typename EntityIterator, typename ComponentIterator>
		class Iterator
		{
			EntityIterator entityIterator;
			ComponentIterator componentIterator;
		public:
			Iterator(EntityIterator entityIterator, ComponentIterator componentIterator)
				: entityIterator(entityIterator), componentIterator(componentIterator) {}

			auto operator*() const
			{
				return std::pair<decltype(*entityIterator), decltype(*componentIterator)>(*entityIterator, *componentIterator);
			}

			Iterator& operator++()
			{
				++entityIterator;
				++componentIterator;
				return *this;
			}

			bool operator!=(const Iterator& other) const
			{
				return entityIterator != other.entityIterator;
			}
		};

		template<typename EntityIterator, typename ComponentIterator>
		class DataView
		{
			EntityIterator entitiesBegin;
			EntityIterator entitiesEnd;
			ComponentIterator componentsBegin;
			//为什么不要end呢？因为entitiesEnd和componentsEnd的长度是一样的，所以可以通过entitiesEnd - entitiesBegin来计算componentsEnd的位置
		public:
			DataView(EntityIterator entitiesBegin, EntityIterator entitiesEnd, ComponentIterator componentsBegin)
				: entitiesBegin(entitiesBegin), entitiesEnd(entitiesEnd), componentsBegin(componentsBegin) {}

			auto begin() const { return Iterator<EntityIterator, ComponentIterator>(entitiesBegin, componentsBegin); }
			auto end() const { return Iterator<EntityIterator, ComponentIterator>(entitiesEnd, componentsBegin + (entitiesEnd - entitiesBegin)); }
		};

		template<typename EntityIterator, typename ComponentIterator>
		class Iterator
		{
			EntityIterator entityIterator;
			ComponentIterator componentIterator;
		public:
			Iterator(EntityIterator entityIterator, ComponentIterator componentIterator)
				: entityIterator(entityIterator), componentIterator(componentIterator) {}

			auto operator*() const
			{
				return std::pair<decltype(*entityIterator), decltype(*componentIterator)>(*entityIterator, *componentIterator);
			}

			Iterator& operator++()
			{
				++entityIterator;
				++componentIterator;
				return *this;
			}

			bool operator!=(const Iterator& other) const
			{
				return entityIterator != other.entityIterator;
			}
		};

		template<typename EntityIterator, typename ComponentIterator>
		class DataView
		{
			EntityIterator entitiesBegin;
			EntityIterator entitiesEnd;
			ComponentIterator componentsBegin;
			//为什么不要end呢？因为entitiesEnd和componentsEnd的长度是一样的，所以可以通过entitiesEnd - entitiesBegin来计算componentsEnd的位置
		public:
			DataView(EntityIterator entitiesBegin, EntityIterator entitiesEnd, ComponentIterator componentsBegin)
				: entitiesBegin(entitiesBegin), entitiesEnd(entitiesEnd), componentsBegin(componentsBegin) {}

			auto begin() const { return Iterator<EntityIterator, ComponentIterator>(entitiesBegin, componentsBegin); }
			auto end() const { return Iterator<EntityIterator, ComponentIterator>(entitiesEnd, componentsBegin + (entitiesEnd - entitiesBegin)); }
		};

	public:
		ComponentStorage() = default;
		~ComponentStorage() override = default;

		// 拷贝
		ComponentStorage(const ComponentStorage&) = default;
		ComponentStorage& operator=(const ComponentStorage&) = default;

		// 移动
		ComponentStorage(ComponentStorage&&) noexcept = default;
		ComponentStorage& operator=(ComponentStorage&&) noexcept = default;

		// 迭代器支持
		auto begin() { return Iterator(entities.begin(), components.begin()); }
		auto end() { return Iterator(entities.end(), components.end()); }

		// 支持左值引用拷贝
		void addTo(Entity entity, const T& component)
		{
			if (has(entity)) return; // 避免重复添加

			entities.push_back(entity);
			try
			{
				components.push_back(component);
			}
			catch (...)
			{
				entities.pop_back();
				throw;
			}
			entityToIndex[entity] = entities.size() - 1;
		}

		// 支持右值引用移动（更高效）
		void addTo(Entity entity, T&& component)
		{
			if (has(entity)) return;

			entities.push_back(entity);
			try
			{
				components.push_back(std::move(component));
			}
			catch (...)
			{
				entities.pop_back();
				throw;
			}
			entityToIndex[entity] = entities.size() - 1;
		}

		// O(1) 复杂度的删除操作
		void remove(Entity entity)
		{
			auto it = entityToIndex.find(entity);
			if (it == entityToIndex.end()) return;

			std::size_t indexToRemove = it->second;
			std::size_t lastIndex = components.size() - 1;

			if (indexToRemove != lastIndex)
			{
				// 将要删除的元素与最后一个元素交换
				Entity lastEntity = entities[lastIndex];

				entities[indexToRemove] = lastEntity;
				components[indexToRemove] = std::move(components[lastIndex]);

				// 更新被移动的那个实体的索引映射
				entityToIndex[lastEntity] = indexToRemove;
			}

			// 弹出最后一个元素
			entities.pop_back();
			components.pop_back();
			entityToIndex.erase(it);
		}

		bool has(Entity entity) const
		{
			return entityToIndex.find(entity) != entityToIndex.end();
		}

		// 返回存储中组件的可修改引用，不复制组件。
		T& get(Entity entity)
		{
			auto it = entityToIndex.find(entity);
			if (it != entityToIndex.end())
			{
				return components[it->second];
			}
			throw std::runtime_error("Component not found for the given entity");
		}

		// const 存储返回只读引用，不复制组件。
		const T& get(Entity entity) const
		{
			auto it = entityToIndex.find(entity);
			if (it != entityToIndex.end())
			{
				return components[it->second];
			}
			throw std::runtime_error("Component not found for the given entity");
		}

		// 按请求顺序返回组件的只读引用包装；返回的 vector 只拥有包装器，不拥有或复制组件。
		std::vector<std::reference_wrapper<const T>> getForEntities(const std::vector<Entity>& requestedEntities) const
		{
			std::vector<std::reference_wrapper<const T>> result;
			result.reserve(requestedEntities.size());
			for (Entity entity : requestedEntities)
			{
				result.emplace_back(std::cref(get(entity)));
			}
			return result;
		}

		// 返回内部实体数组的只读引用；与 getAllComponents() 的元素按索引对应。
		const std::vector<Entity>& getEntities() const
		{
			return entities;
		}

		// 按值返回非拥有视图；可通过视图访问实体及可修改组件引用，不复制底层数据。
		auto getDataView() {
			return DataView(entities.begin(), entities.end(), components.begin());
		}

		// 返回内部组件数组的只读引用；与 getEntities() 的元素按索引对应。
		const std::vector<T>& getAllComponents() {
			return components;
		}

		// 按值返回只读的非拥有视图，每项包含实体引用和组件只读引用。
		auto getAllComponentsWithEntities() const
		{
			return DataView(entities.cbegin(), entities.cend(), components.cbegin());
		}
	};
	using PositionComponent = glm::vec3;
	using VelocityComponent = glm::vec3;
	using PositionComponentStorage = ComponentStorage<PositionComponent>;
}
