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

	_window   = Window::Create(_name);
	_renderer = Renderer::Create(_window.get());
}

Engine::~Engine() {}

void Engine::Run() {
	while (!_window->ShouldClose()) {
		glfwPollEvents();

		_renderer->BeginFrame();
		//_renderer.UpdateEditor(m_currentScene.get());
		// UpdateRigidBodyTransform();
		// m_renderer.UpdateAnimations(m_currentScene->GetRegistry());
		_renderer->RenderpassEntities(/*m_currentScene->GetRegistry(), m_mainCamera*/);
		// m_renderer.RenderEntities(m_currentScene.get());
		// m_renderer.RenderEnv(m_currentScene->GetRegistry());
		_renderer->RenderpassGUI();

		// m_renderer.RenderpassGUI();
		_renderer->EndScene();
	}
}
}  // namespace RV