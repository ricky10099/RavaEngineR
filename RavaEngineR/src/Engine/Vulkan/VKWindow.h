#pragma once

#include "Engine/System/Window.h"

namespace VK {
class Window : public RV::Window {
   public:
	Window(std::string_view title, bool isFullScreen, u32 width, u32 height);
	virtual ~Window() override;

	NO_COPY(Window)

	bool ShouldClose() const override { return glfwWindowShouldClose(_window); }
	void ResetWindowResizedFlag() { _resized = false; }

	void* GetBackendWindow() const override { return (void*)_window; }
	VkExtent2D GetExtent() { return {static_cast<u32>(_width), static_cast<u32>(_height)}; }
	bool IsWindowResized() const { return _resized; }

   private:
	static void FramebufferResizeCallback(GLFWwindow* window, i32 width, i32 height);
	void InitWindow();

   private:
	GLFWwindow* _window;

	// Properties
	std::string_view _title;
	u32 _width  = 1280;
	u32 _height = 720;
	bool _isFullscreen;
	bool _resized = false;
};
}  // namespace VK