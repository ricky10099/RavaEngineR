#include "rvpch.h"

#include "Engine/Engine.h"
#include "Engine/System/Window.h"
#include "Engine/System/Renderer.h"

namespace RV {
Engine* Engine::Instance = nullptr;

Engine::Engine() {
	if (Instance == nullptr) {
		Instance = this;
	} else {
		ENGINE_CRITICAL("Engine already running!");
	}

	_window = Window::Create(_name);
}

Engine::~Engine() {
}

void Engine::Run() {
	while (!_window->ShouldClose()) {
		glfwPollEvents();
	}
}
}  // namespace RV