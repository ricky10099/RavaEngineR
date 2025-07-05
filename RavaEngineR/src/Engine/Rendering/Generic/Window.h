#pragma once

namespace RV {
class Window {
   public:
	Window() {}
	virtual ~Window() {}

	virtual bool ShouldClose() const = 0;

	virtual void* GetBackendWindow() const = 0;

	static Unique<Window> Create(std::string_view title, bool isFullscreen = _defaultIsFullscreen, u32 width = _defaultWidth, u32 height = _defaultHeight);

private:
	static const u32 _defaultWidth = 1280;
	static const u32 _defaultHeight = 720;
	static const bool _defaultIsFullscreen = false;
};
}