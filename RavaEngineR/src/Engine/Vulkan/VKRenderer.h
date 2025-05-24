#pragma once

#include "Engine/System/Renderer.h"

namespace VK {
class Window;
class Swapchain;
class RenderPass;
class Buffer;
class DescriptorPool;
class Renderer : public RV::Renderer {
   public:
	static Unique<DescriptorPool> GlobalDescriptorPool;
   public:
	Renderer(Window* window);
	virtual ~Renderer() override = default;

	NO_COPY(Renderer)

	void Init();
	void BeginFrame();
	void EndFrame();
	void Begin3DRenderPass();
	void BeginGUIRenderPass();
	void EndRenderPass();
	void RenderpassGUI(/*Camera* camera*/);
	void EndScene();

	int GetFrameIndex() const;
	VkCommandBuffer GetCurrentCommandBuffer() const;
	std::shared_ptr<RenderPass> GetRenderPass() { return _renderPass; }
	u32 GetImageCount() { return static_cast<u32>(_swapchain->ImageCount()); }
	// float GetAmbientLightIntensity() const { return m_ambientLightIntensity; }
	u32 GetFrameCounter() const { return _frameCounter; }
	float GetAspectRatio() const { return _swapchain->ExtentAspectRatio(); }
	u32 GetContextWidth() const { return _swapchain->Width(); }
	u32 GetContextHeight() const { return _swapchain->Height(); }
	bool FrameInProgress() const { return _frameInProgress; }

   private:
	Window* _window;
	Unique<Swapchain> _swapchain;
	Shared<RenderPass> _renderPass;
	std::vector<VkCommandBuffer> _commandBuffers;
	VkCommandBuffer _currentCommandBuffer            = VK_NULL_HANDLE;
	VkDescriptorSetLayout _globalDescriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> _globalDescriptorSets{MAX_FRAMES_SYNC};
	std::vector<Unique<Buffer>> _uniformBuffers{MAX_FRAMES_SYNC};

	u32 _frameCounter = 0;
	u32 _currentImageIndex = 0;
	i32 _currentFrameIndex = 0;
	bool _frameInProgress = false;
	FrameInfo m_frameInfo{};

	private:
	void CreateCommandBuffers();
	void FreeCommandBuffers();
	void RecreateSwapChain();
	void RecreateRenderpass();
	void Recreate();
};
}  // namespace VK