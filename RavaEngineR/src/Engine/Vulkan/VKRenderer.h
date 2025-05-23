#pragma once

namespace VK {
class Renderer : public RV::Renderer {
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

   private:
	Unique<SwapChain> _swapChain;
	Unique<RenderPass> _renderPass;
	std::vector<VkCommandBuffer> _commandBuffers;
	VkCommandBuffer _currentCommandBuffer            = VK_NULL_HANDLE;
	VkDescriptorSetLayout _globalDescriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> _globalDescriptorSets{MAX_FRAMES_SYNC};
	std::vector<std::unique_ptr<Buffer>> _uniformBuffers{MAX_FRAMES_SYNC};

	private:
	void CreateCommandBuffers();
	void FreeCommandBuffers();
	void RecreateSwapChain();
	void RecreateRenderpass();
	void Recreate();
};
}  // namespace VK