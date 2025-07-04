#include "rvpch.h"

#include <stb/stb_image.h>

#include "Engine/Rendering/Vulkan/VKWindow.h"
#include "Engine/Rendering/Vulkan/VKContext.h"

namespace VK {
Window::Window(std::string_view title, bool isFullScreen, u32 width, u32 height)
	: _title(title)
	, _isFullscreen(isFullScreen)
	, _width(width)
	, _height(height) {
	InitWindow();

	Context::Instance = std::make_unique<Context>(this);
}

Window::~Window() {
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void Window::InitWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWmonitor* primaryMonitor  = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);

	if (_isFullscreen) {
		_window = glfwCreateWindow(videoMode->width, videoMode->height, _title.data(), primaryMonitor, nullptr);

	} else {
		int monitorX, monitorY;
		glfwGetMonitorPos(primaryMonitor, &monitorX, &monitorY);
		_window = glfwCreateWindow(_width, _height, _title.data(), nullptr, nullptr);
		glfwSetWindowPos(_window, monitorX + (videoMode->width - _width) / 2, monitorY + (videoMode->height - _height) / 2);
		glfwShowWindow(_window);
	}

	if (!glfwVulkanSupported()) {
		ENGINE_CRITICAL("GLFW: Vulkan not supported!");
	}

	GLFWimage icon{};
	int channels;
	std::string iconPathStr = "Assets/System/Images/Rava.png";
	icon.pixels             = stbi_load(iconPathStr.c_str(), &icon.width, &icon.height, &channels, 4);
	if (icon.pixels) {
		glfwSetWindowIcon(_window, 1, &icon);
		stbi_image_free(icon.pixels);
	}

	glfwSetWindowUserPointer(_window, this);
	glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
}

void Window::FramebufferResizeCallback(GLFWwindow* glfwWindow, i32 width, i32 height) {
	auto window      = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	window->_resized = true;
	window->_width   = width;
	window->_height  = height;
}
}  // namespace VK