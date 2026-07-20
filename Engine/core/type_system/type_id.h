#pragma once
namespace _type_id {
	size_t nextTypeId();
}

using namespace _type_id;
namespace neon::core::type_system
{
	template<typename T>
	size_t getTypeId()
	{
		static size_t id = nextTypeId();
		return id;
	}
}