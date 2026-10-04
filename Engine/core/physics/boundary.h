#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <string_view>

namespace neon::core::physics
{
	/// 边界的种类。启动时确定一次，运行期间不切换（见 PhysicsSystem::initializeBoundary）。
	enum class BoundaryType
	{
		Wrap,	// 循环：从对面出现，速度不变，空间首尾相接（环面）
		Bounce	// 反弹：贴在边界上，法向速度反向
	};

	/// 可见区是 [-halfExtent, halfExtent)²，两种边界共用的几何参数
	inline constexpr float defaultHalfExtent = 1.0f;

	/// 边界接口：只负责一件事——处理走到边界的粒子。
	/// 位置和速度都交给实现去改（循环边界只改位置，反弹边界还要改速度），
	/// 因此新增一种边界不需要动物理系统，物理系统也不需要知道具体是哪种边界。
	/// z 轴目前不受约束（和原来的实现一致），将来要加三维边界时在这里扩展。
	class Boundary
	{
	public:
		virtual ~Boundary() = default;

		/// 处理一颗粒子的边界情况（在位置积分之后调用）。
		/// @return 被处理的坐标分量个数（0 表示没有碰到边界），用于诊断统计
		virtual int apply(glm::vec3& position, glm::vec3& velocity) const = 0;

		virtual BoundaryType getType() const = 0;
		/// 日志里用的名字
		virtual const char* getName() const = 0;

		float getHalfExtent() const { return halfExtent; }

	protected:
		explicit Boundary(float halfExtent) : halfExtent(halfExtent) {}

	private:
		const float halfExtent;
	};

	/// 循环边界：越过一侧就从另一侧出现，速度不变
	class WrapBoundary final : public Boundary
	{
	public:
		explicit WrapBoundary(float halfExtent = defaultHalfExtent) : Boundary(halfExtent) {}

		int apply(glm::vec3& position, glm::vec3& velocity) const override;
		BoundaryType getType() const override { return BoundaryType::Wrap; }
		const char* getName() const override { return "wrap (torus)"; }
	};

	/// 反弹边界：碰到边界就贴住，并把该方向的速度反向
	class BounceBoundary final : public Boundary
	{
	public:
		explicit BounceBoundary(float halfExtent = defaultHalfExtent) : Boundary(halfExtent) {}

		int apply(glm::vec3& position, glm::vec3& velocity) const override;
		BoundaryType getType() const override { return BoundaryType::Bounce; }
		const char* getName() const override { return "bounce"; }
	};

	/// 按类型创建边界：只在启动时调用一次
	std::unique_ptr<Boundary> createBoundary(BoundaryType type, float halfExtent = defaultHalfExtent);

	/// 把配置/命令行里的名字转成边界类型；无法识别时返回 false（type 不变）
	bool parseBoundaryType(std::string_view name, BoundaryType& type);

	/// 供日志/帮助信息使用：所有可用的边界名字
	std::string_view availableBoundaryNames();

	/// 类型对应的名字（写法与 --boundary 接受的值一致）
	std::string_view boundaryTypeName(BoundaryType type);
}
