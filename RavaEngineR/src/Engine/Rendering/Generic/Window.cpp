#include "rvpch.h"

#include "Engine/Engine.h"
#include "Engine/Rendering/Generic/Window.h"
#include "Engine/Rendering/Generic/RendererAPI.h"

#include "Engine/Rendering/Vulkan/VKWindow.h"

namespace RV {

std::unique_ptr<Window> Window::Create(std::string_view title, bool isFullscreen, u32 width, u32 height) {
	std::unique_ptr<Window> window;

	switch (RendererAPI::GetAPI()) {
		case RendererAPI::VULKAN:
			window = std::make_unique<VK::Window>(title, isFullscreen, width, height);
			break;
		default:
			window = nullptr;
			break;
	}

	return window;
}
}  // namespace RV
