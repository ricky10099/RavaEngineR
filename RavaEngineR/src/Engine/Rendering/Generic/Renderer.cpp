#include "rvpch.h"

#include "Engine/Rendering/Generic/Renderer.h"
#include "Engine/Rendering/Generic/RendererAPI.h"

#include "Engine/Rendering/Vulkan/VKWindow.h"
#include "Engine/Rendering/Vulkan/VKRenderer.h"

namespace RV {
Unique<Renderer> Renderer::Create(void* window) {
	Unique<Renderer> renderer;

	switch (RendererAPI::GetAPI()) {
		case RendererAPI::VULKAN:
			renderer = std::make_unique<VK::Renderer>(static_cast<VK::Window*>(window));
			break;
		default:
			renderer = nullptr;
			break;
	}

	return renderer;
}
}  // namespace RV