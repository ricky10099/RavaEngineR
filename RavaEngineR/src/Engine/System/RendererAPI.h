#pragma once

namespace RV {
class RendererAPI {
   public:
	enum APIs {
		VULKAN = 0,
		DX12
	};

   public:
	static APIs GetAPI() { return s_api; }

   private:
	static APIs s_api;
};
}  // namespace RV
