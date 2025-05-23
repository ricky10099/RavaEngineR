#pragma once

namespace RV {
class Window {
   public:
	Window() {}
	virtual ~Window() {}

	virtual bool ShouldClose() const = 0;

	virtual void* GetBackendWindow() const = 0;

	static Unique<Window> Create(std::string_view title);
};
}