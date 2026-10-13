#include "core/engine.h"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char* argv[])
{
#ifdef _WIN32
	std::system("chcp 65001 > nul");
#endif

	try {
		neon::core::Engine engine;
		if (!engine.init(argc > 0 ? argv[0] : "")) {
			return EXIT_FAILURE;
		}
		engine.run();
	}
	catch (const std::exception& e) {
		std::cerr << "Engine failed: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
