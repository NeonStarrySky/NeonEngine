#include "type_id.h"

namespace _type_id {
	std::size_t nextTypeId()
	{
		static std::size_t id = 0;
		return id++;
	}
}
