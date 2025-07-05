#pragma once

namespace VK {
class Swapchain {
   public:
	Swapchain(VkExtent2D windowExtent);
	Swapchain(VkExtent2D windowExtent, Shared<Swapchain> oldSwapchain);
	~Swapchain();

	NO_COPY(Swapchain)

	VkResult AcquireNextImage(u32* imageIndex);
	VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, u32* imageIndex);
	bool CompareFormats(const Swapchain& swapchain) const;
	void TransitImageLayout(
		VkImageLayout orgLayout, VkImageLayout trgLayout, u32 currentImageIndex, VkCommandBuffer commandBuffer
	);

	VkImageView GetImageView(i32 index) { return _swapchainImageViews[index]; }
	size_t ImageCount() { return _swapchainImages.size(); }
	VkFormat GetSwapChainImageFormat() const { return _swapchainImageFormat; }
	VkExtent2D GetSwapChainExtent() const { return _swapchainExtent; }
	u32 Width() const { return _swapchainExtent.width; }
	u32 Height() const { return _swapchainExtent.height; }

	float ExtentAspectRatio() const {
		return static_cast<float>(_swapchainExtent.width) / static_cast<float>(_swapchainExtent.height);
	}

   private:
	VkFormat _swapchainImageFormat;
	VkExtent2D _swapchainExtent;

	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;
	Shared<Swapchain> _oldSwapchain;
	VkExtent2D _windowExtent;

	VkSwapchainKHR _swapchain;

	std::vector<VkSemaphore> _availableSemaphores;
	std::vector<VkSemaphore> _finishedSemaphores;
	std::vector<VkFence> _imagesFences;
	std::vector<VkFence> _renderFences;
	size_t _currentFrame = 0;

   private:
	void Init();
	void CreateSwapchain();
	void CreateSwapchainImageViews();
	void CreateSyncObjects();

	// Helper functions
	VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
};
}  // namespace VK