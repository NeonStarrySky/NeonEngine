#pragma once

#include "entity.h"
#include <cstdint>

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

	template<typename T>
	class ComponentStorage : public ComponentStorageBase
	{
		//static_assert(std::is_trivially_copyable_v<T>, "Component type must be trivially copyable");

		std::vector<T> data;
		std::unordered_map<Entity, std::size_t> entityToIndex;
		// 引入反向映射：通过数组索引快速找到对应的 Entity，用于 Swap-and-Pop 优化
		std::vector<Entity> indexToEntity;

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
		auto begin() { return data.begin(); }
		auto end() { return data.end(); }

		// 支持左值引用拷贝
		void addTo(Entity entity, const T& component)
		{
			if (has(entity)) return; // 避免重复添加

			data.push_back(component);
			entityToIndex[entity] = data.size() - 1;

			indexToEntity.push_back(entity);

		}

		// 支持右值引用移动（更高效）
		void addTo(Entity entity, T&& component)
		{
			if (has(entity)) return;

			data.push_back(std::move(component));
			entityToIndex[entity] = data.size() - 1;

			indexToEntity.push_back(entity);
		}

		// O(1) 复杂度的删除操作
		void remove(Entity entity)
		{
			auto it = entityToIndex.find(entity);
			if (it == entityToIndex.end()) return;

			std::size_t indexToRemove = it->second;
			std::size_t lastIndex = data.size() - 1;

			if (indexToRemove != lastIndex)
			{
				// 将要删除的元素与最后一个元素交换
				T lastComponent = std::move(data[lastIndex]);
				Entity lastEntity = indexToEntity[lastIndex];

				data[indexToRemove] = std::move(lastComponent);
				indexToEntity[indexToRemove] = lastEntity;

				// 更新被移动的那个实体的索引映射
				entityToIndex[lastEntity] = indexToRemove;
			}

			// 弹出最后一个元素
			data.pop_back();
			indexToEntity.pop_back();
			entityToIndex.erase(it);
		}

		bool has(Entity entity) const
		{
			return entityToIndex.find(entity) != entityToIndex.end();
		}

		T& get(Entity entity)
		{
			auto it = entityToIndex.find(entity);
			if (it != entityToIndex.end())
			{
				return data[it->second];
			}
			throw std::runtime_error("Component not found for the given entity");
		}

		const T& get(Entity entity) const
		{
			auto it = entityToIndex.find(entity);
			if (it != entityToIndex.end())
			{
				return data[it->second];
			}
			throw std::runtime_error("Component not found for the given entity");
		}
	};
	using PositionComponent = glm::vec3;
	using VelocityComponent = glm::vec3;
	using PositionComponentStorage = ComponentStorage<PositionComponent>;
}