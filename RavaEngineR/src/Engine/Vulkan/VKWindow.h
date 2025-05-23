#pragma once

#include "Engine/System/Window.h"

namespace VK {
class Window : public RV::Window {
   public:
	Window(std::string_view title);
	virtual ~Window() override;

	NO_COPY(Window)

	bool ShouldClose() const override { return glfwWindowShouldClose(_window); }

	void* GetBackendWindow() const override { return (void*)_window; }
	bool IsWindowResized() const { return _resized; }

   private:
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
	void InitWindow();

   private:
	GLFWwindow* _window;

	// Properties
	std::string_view _title;
	u32 _width    = 1280;
	u32 _height   = 720;
	bool _resized = false;
};
}  // namespace VK