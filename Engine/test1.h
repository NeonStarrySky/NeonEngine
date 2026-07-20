#pragma once

#include "core/type_system/type_id.h"

#include <iostream>

void testFuc() {
	std::cout << neon::core::type_system::getTypeId<int>() << std::endl;
	std::cout << neon::core::type_system::getTypeId<float>() << std::endl;
	std::cout << neon::core::type_system::getTypeId<int>() << std::endl;
}