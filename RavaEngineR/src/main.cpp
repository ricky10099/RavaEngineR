#include "rvpch.h"
#include "Engine/Engine.h"
#include "Engine/System/Log.h"

int main() {
	RV::Engine engine{};

	try {
		engine.Run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';		
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}