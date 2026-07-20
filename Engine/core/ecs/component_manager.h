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

		template<typename T>
		void registerComponent()
		{

			static_assert(std::is_base_of_v<ComponentStorageBase, T>);// 确保 T 是 ComponentStorageBase 的派生类

			auto typeId = core::type_system::getTypeId<T>();

			if (indexMap.contains(typeId)) return;// 防止重复注册

			T temp; // 创建一个临时对象以获取其类型 ID
			temp.setTypeId(typeId);// 设置类型 ID

			data.push_back(std::make_unique<T>(std::move(temp)));// 将临时对象移动到 vector 中


			indexMap[typeId] = data.size() - 1;// 记录组件类型对应的索引
		}

		template<typename T>
		void unregisterComponent()
		{

			static_assert(std::is_base_of_v<ComponentStorageBase, T>);// 确保 T 是 ComponentStorageBase 的派生类

			auto typeId = core::type_system::getTypeId<T>();
			auto it = indexMap.find(typeId);

			// 1. 安全检查：如果该组件根本没有注册，直接返回
			if (it == indexMap.end()) {
				return;
			}

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
		T& get()
		{

			static_assert(std::is_base_of_v<ComponentStorageBase, T>);// 确保 T 是 ComponentStorageBase 的派生类

			auto typeId = core::type_system::getTypeId<T>();
			auto it = indexMap.find(typeId);
			assert(it != indexMap.end());// 确保组件类型已注册
			return static_cast<T&>(*data[it->second]);
		}

		template<typename T>
		void add(Entity entity, const T& component)
		{
			get<ComponentStorage<T>>().add(entity, component);
		}
	};
}