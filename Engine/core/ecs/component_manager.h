#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "component_storage.h"
#include "core/type_system/type_id.h"
#include "entity.h"

namespace neon::core::ecs
{
	// 组件管理器，用于注册和管理不同类型的组件存储
	class ComponentManager
	{
		std::vector<std::unique_ptr<ComponentStorageBase>> data;
		std::unordered_map<std::size_t, std::size_t> indexMap;
	public:
		ComponentManager() = default;
		~ComponentManager() = default;

		ComponentManager(ComponentManager&&) = default;
		ComponentManager& operator=(ComponentManager&&) = default;

		ComponentManager(const ComponentManager&) = delete;
		ComponentManager& operator=(const ComponentManager&) = delete;

		//注册组件（而非组件存储器！！
		template<typename T>
		ComponentStorage<T>& registerComponent()
		{

			static_assert(std::is_class_v<T>, "Template parameter T must be a struct or class!");

			auto typeId = core::type_system::getTypeId<T>();

			if (indexMap.contains(typeId)) return *static_cast<ComponentStorage<T>*>(data[indexMap[typeId]].get());// 如果已经注册过该组件类型，则直接返回对应的存储器

			ComponentStorage<T> temp;
			temp.setTypeId(typeId);// 设置类型 ID

			data.push_back(std::make_unique<ComponentStorage<T>>(std::move(temp)));// 将临时对象移动到 vector 中


			indexMap[typeId] = data.size() - 1;// 记录组件类型对应的索引

			return *static_cast<ComponentStorage<T>*>(data.back().get());
		}

		template<typename T>
		void unregisterComponent()
		{

			static_assert(std::is_class_v<T>, "Template parameter T must be a struct or class!");

			auto typeId = core::type_system::getTypeId<T>();
			auto it = indexMap.find(typeId);

			// 使用未注册的组件类型时，抛出异常或断言失败
			assert
			(
				it != indexMap.end()
				&&
				(std::string("Type not registered: ") + std::string(typeid(T).name())).c_str()
			);

			std::size_t targetIndex = it->second;// 获取要删除的组件在 data 中的索引
			std::size_t lastIndex = data.size() - 1;// 获取最后一个组件的索引

			// 2. 如果要删除的不是最后一个元素，则进行 Swap-and-Pop
			if (targetIndex != lastIndex) {
				// 获取原本处于末尾的组件的类型 ID（我们需要更新它的索引映射）
				// 注意：这里需要通过底层的 ComponentStorageBase 获取其真实的 TypeId。
				// 假设你的 ComponentStorageBase 类中有一个虚函数 getTypeId()
				std::size_t lastComponentTypeId = data[lastIndex]->getTypeId();

				// 将末尾的元素移动到要删除的位置（覆盖它，释放 targetIndex 处的内存）
				data[targetIndex] = std::move(data[lastIndex]);

				// 更新原本在末尾的元素在 map 中的索引
				indexMap[lastComponentTypeId] = targetIndex;
			}

			// 3. 弹出末尾元素（如果是最后一个元素，直接弹出；如果是中间元素，此时它已被移到末尾）
			data.pop_back();

			// 4. 从 map 中移除被注销组件的记录
			indexMap.erase(it);
		}

		template<typename T>
		ComponentStorage<T>& get()
		{

			static_assert(std::is_class_v<T>, "Template parameter T must be a struct or class!");

			auto typeId = core::type_system::getTypeId<T>();
			auto it = indexMap.find(typeId);

			assert
			(
				it != indexMap.end()
				&&
				(std::string("Type not registered: ") + std::string(typeid(T).name())).c_str()
			);// 确保组件类型已注册
			return static_cast<ComponentStorage<T>&>(*data[it->second]);
		}

		template<typename T>
		void add(Entity entity, const T& component)
		{
			get<ComponentStorage<T>>().add(entity, component);
		}
	};
}