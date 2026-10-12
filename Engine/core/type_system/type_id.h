#pragma once

#include <cstddef>

namespace _type_id {
	std::size_t nextTypeId();
}

using namespace _type_id;
namespace neon::core::type_system
{
	template<typename T>
	std::size_t getTypeId()
	{
		static std::size_t id = nextTypeId();
		return id;
	}
}
