#pragma once

namespace RV {
class Window;
class Renderer;
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
};
}  // namespace RV