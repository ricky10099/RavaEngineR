#pragma once

namespace RV {
class Renderer {
   public:
	virtual ~Renderer() = default;

	static Unique<Renderer> Create(void* window);
};
}  // namespace RV