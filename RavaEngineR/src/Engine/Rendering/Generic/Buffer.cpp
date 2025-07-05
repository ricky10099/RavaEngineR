#include "rvpch.h"

#include "Engine/Rendering/Generic/Buffer.h"
#include "Engine/Rendering/Generic/RendererAPI.h"

#include "Engine/Rendering/Vulkan/VKBuffer.h"

namespace RV {
	Shared<Buffer> Buffer::Create(u32 size, BufferUsage bufferUsage) {
		Shared<Buffer> buffer;

		switch (RendererAPI::GetAPI()) {
		case RendererAPI::VULKAN:
				buffer = std::make_shared<VK::Buffer>(size, bufferUsage);
			break;
		default:
			buffer = nullptr;
			break;
		}

		return buffer;
	}
}