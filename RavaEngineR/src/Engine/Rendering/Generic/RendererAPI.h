#pragma once

namespace RV {
class RendererAPI {
   public:
	enum APIs {
		VULKAN = 0,
		DX12
	};

   public:
	static APIs GetAPI() { return _api; }

   private:
	static APIs _api;
};
}  // namespace RV
