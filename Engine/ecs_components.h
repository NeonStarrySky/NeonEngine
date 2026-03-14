#pragma once

#include "entity.h"

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <type_traits>
#include <vector>


namespace neon::core::ecs
{
	// 组件存储类：每种组件类型一个数组，索引对应实体ID
	template<typename T>
	class ComponentStorage
	{
		std::vector<T> data;
	public:
		T& operator[](Entity entity) { return data[static_cast<uint32_t>(entity)]; }
	};

	using PositionComponents = ComponentStorage<glm::vec3>;
	using VelocityComponents = ComponentStorage<glm::vec3>;
	using ForceComponents = ComponentStorage<glm::vec3>;

	class ComponentManager
	{
		PositionComponents positionComponents;
		VelocityComponents velocityComponents;
		ForceComponents forceComponents;
	public:
		template<typename T>
		T& get() {
			if constexpr (std::is_same_v<T, PositionComponents>) {
				return positionComponents;
			}
			else if constexpr (std::is_same_v<T, VelocityComponents>) {
				return velocityComponents;
			}
			else if constexpr (std::is_same_v<T, ForceComponents>) {
				return forceComponents;
			}
			else {
				static_assert(false, "Unsupported component type");
			}
		}
	};
}