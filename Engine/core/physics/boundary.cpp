#include "boundary.h"

#include <cmath>

namespace neon::core::physics
{
	namespace
	{
		/// 把坐标折回 [-halfExtent, halfExtent)。
		/// 用取模而不是“越界就减一个周期”，这样一帧内跨过多个周期（高速粒子）也能得到正确结果。
		///
		/// 注意必须先判断“已经在范围内就直接返回”：value + halfExtent 会把小于 halfExtent 的坐标
		/// 提升到 [halfExtent, 2 * halfExtent)，那个区间的 ulp 比原区间大一倍，再减回来就不是原值了
		/// （float32 下约一半的坐标会被改动 1 ulp，结果恰好等于按量级 halfExtent~2*halfExtent 的
		/// 绝对格子取整）。那会让“是否环绕”的判断把舍入噪声当成环绕，还会持续给粒子施加微小位移，
		/// 所以范围内的坐标必须原样返回。
		float wrapCoordinate(float value, float halfExtent)
		{
			if (value >= -halfExtent && value < halfExtent)
			{
				return value;
			}

			const float period = 2.0f * halfExtent;
			float shifted = std::fmod(value + halfExtent, period);
			if (shifted < 0.0f) shifted += period;
			return shifted - halfExtent;
		}
	}

	int WrapBoundary::apply(glm::vec3& position, glm::vec3&) const
	{
		const float wrappedX = wrapCoordinate(position.x, getHalfExtent());
		const float wrappedY = wrapCoordinate(position.y, getHalfExtent());

		int handled = 0;
		if (wrappedX != position.x) ++handled;
		if (wrappedY != position.y) ++handled;

		position.x = wrappedX;
		position.y = wrappedY;
		return handled;
	}

	int BounceBoundary::apply(glm::vec3& position, glm::vec3& velocity) const
	{
		const float halfExtent = getHalfExtent();
		int handled = 0;

		if (position.x < -halfExtent) {
			position.x = -halfExtent;
			if (velocity.x < 0.0f) velocity.x = -velocity.x;
			++handled;
		} else if (position.x > halfExtent) {
			position.x = halfExtent;
			if (velocity.x > 0.0f) velocity.x = -velocity.x;
			++handled;
		}

		if (position.y < -halfExtent) {
			position.y = -halfExtent;
			if (velocity.y < 0.0f) velocity.y = -velocity.y;
			++handled;
		} else if (position.y > halfExtent) {
			position.y = halfExtent;
			if (velocity.y > 0.0f) velocity.y = -velocity.y;
			++handled;
		}

		return handled;
	}

	std::unique_ptr<Boundary> createBoundary(BoundaryType type, float halfExtent)
	{
		switch (type)
		{
		case BoundaryType::Bounce:
			return std::make_unique<BounceBoundary>(halfExtent);
		case BoundaryType::Wrap:
		default:
			return std::make_unique<WrapBoundary>(halfExtent);
		}
	}

	bool parseBoundaryType(std::string_view name, BoundaryType& type)
	{
		if (name == "wrap" || name == "torus" || name == "periodic")
		{
			type = BoundaryType::Wrap;
			return true;
		}
		if (name == "bounce" || name == "reflect" || name == "wall")
		{
			type = BoundaryType::Bounce;
			return true;
		}
		return false;
	}

	std::string_view availableBoundaryNames()
	{
		return "wrap|bounce";
	}

	std::string_view boundaryTypeName(BoundaryType type)
	{
		switch (type)
		{
		case BoundaryType::Bounce:
			return "bounce";
		case BoundaryType::Wrap:
		default:
			return "wrap";
		}
	}
}
