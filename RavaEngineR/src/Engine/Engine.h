#pragma once

namespace RV {
class Window;
class Renderer;
class Camera;
class Engine {
   public:
	static Engine* Instance;
	glm::vec4 ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 0.0f);

   public:
	Engine();
	~Engine();

	NO_COPY(Engine)

	void Run();

   private:
	Log _logger;
	std::string_view _name = "Rava Engine R";
	Unique<Window> _window;
	Unique<Renderer> _renderer;

	float _gamma    = 2.0f;
	float _exposure = 1.0f;

	Camera _mainCamera{};
	Camera _editorCamera;
	glm::vec3 _editorCameraPosition   = {0.0f, 2.0f, 2.0f};
	glm::vec3 _editorCameraRotation   = {0.0f, 0.0f, 0.0f};
	glm::vec3 _editorCameraForward    = {0.0f, 0.0f, -1.0f};
	glm::vec3 _editorCameraRight      = {-_editorCameraForward.z, 0.0f, _editorCameraForward.x};
	glm::vec3 _editorCameraUp         = {0.0f, 1.0f, 0.0f};
	glm::vec2 _mouseRotateStartPos    = {0.0f, 0.0f};
	glm::vec2 _mouseTranslateStartPos = {0.0f, 0.0f};
};
}  // namespace RV