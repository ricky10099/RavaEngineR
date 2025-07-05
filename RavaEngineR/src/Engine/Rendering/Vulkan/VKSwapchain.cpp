#include "rvpch.h"

#include "Engine/Rendering/Vulkan/VKSwapchain.h"
#include "Engine/Rendering/Vulkan/VKUtils.h"

namespace VK {
Swapchain::Swapchain(VkExtent2D extent)
	: _windowExtent(extent) {
	Init();
}

Swapchain::Swapchain(VkExtent2D extent, Shared<Swapchain> oldSwapchain)
	: _windowExtent(extent)
	, _oldSwapchain(oldSwapchain) {
	Init();
	_oldSwapchain.reset();
}

Swapchain::~Swapchain() {
	for (auto imageView : _swapchainImageViews) {
		vkDestroyImageView(Context::Instance->GetLogicalDevice(), imageView, nullptr);
	}

	if (_swapchain != nullptr) {
		vkDestroySwapchainKHR(Context::Instance->GetLogicalDevice(), _swapchain, nullptr);
		_swapchain = nullptr;
	}

	for (size_t i = 0; i < MAX_FRAMES_SYNC; i++) {
		vkDestroySemaphore(Context::Instance->GetLogicalDevice(), _finishedSemaphores[i], nullptr);
		vkDestroySemaphore(Context::Instance->GetLogicalDevice(), _availableSemaphores[i], nullptr);
		vkDestroyFence(Context::Instance->GetLogicalDevice(), _renderFences[i], nullptr);
	}
}

void Swapchain::Init() {
	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateSyncObjects();
}

void Swapchain::CreateSwapchain() {
	SwapChainSupportDetails swapChainSupport = Context::Instance->GetSwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(swapChainSupport.Formats);
	VkPresentModeKHR presentMode     = ChoosePresentMode(swapChainSupport.PresentModes);
	VkExtent2D extent                = ChooseExtent(swapChainSupport.Capabilities);

	u32 imageCount = swapChainSupport.Capabilities.minImageCount + 1;
	if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount) {
		imageCount = swapChainSupport.Capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface                  = Context::Instance->GetSurface();
	createInfo.minImageCount            = imageCount;
	createInfo.imageFormat              = surfaceFormat.format;
	createInfo.imageColorSpace          = surfaceFormat.colorSpace;
	createInfo.imageExtent              = extent;
	createInfo.imageArrayLayers         = 1;
	createInfo.imageUsage   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode    = presentMode;
	createInfo.clipped        = VK_TRUE;

	QueueFamilyIndices& indices = Context::Instance->GetPhysicalQueueFamilies();
	u32 queueFamilyIndices[]    = {static_cast<u32>(indices.GraphicsFamily), static_cast<u32>(indices.PresentFamily)};

	if (indices.GraphicsFamily != indices.PresentFamily) {
		createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices   = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices   = nullptr;
	}

	createInfo.oldSwapchain = (_oldSwapchain == nullptr ? VK_NULL_HANDLE : _oldSwapchain->_swapchain);

	VkResult result = vkCreateSwapchainKHR(Context::Instance->GetLogicalDevice(), &createInfo, nullptr, &_swapchain);
	VK_CHECK(result, "Failed to create swap chain!");

	vkGetSwapchainImagesKHR(Context::Instance->GetLogicalDevice(), _swapchain, &imageCount, nullptr);
	_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(Context::Instance->GetLogicalDevice(), _swapchain, &imageCount, _swapchainImages.data());

	_swapchainImageFormat = surfaceFormat.format;
	_swapchainExtent      = extent;
}

void Swapchain::CreateSwapchainImageViews() {
	_swapchainImageViews.resize(_swapchainImages.size());
	for (size_t i = 0; i < _swapchainImages.size(); i++) {
		CreateImageView(_swapchainImages[i], _swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, _swapchainImageViews[i]);
	}
}

void Swapchain::CreateSyncObjects() {
	_availableSemaphores.resize(MAX_FRAMES_SYNC);
	_finishedSemaphores.resize(MAX_FRAMES_SYNC);
	_renderFences.resize(MAX_FRAMES_SYNC);
	_imagesFences.resize(ImageCount(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_SYNC; i++) {
		if (vkCreateSemaphore(Context::Instance->GetLogicalDevice(), &semaphoreInfo, nullptr, &_availableSemaphores[i])
				!= VK_SUCCESS
			|| vkCreateSemaphore(Context::Instance->GetLogicalDevice(), &semaphoreInfo, nullptr, &_finishedSemaphores[i])
				   != VK_SUCCESS
			|| vkCreateFence(Context::Instance->GetLogicalDevice(), &fenceInfo, nullptr, &_renderFences[i]) != VK_SUCCESS) {
			ENGINE_CRITICAL("Failed to create Synchronization objects for a frame!");
		}
	}
}

VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			|| availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB
				   && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR Swapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			ENGINE_INFO("Present mode: Mailbox");
			return availablePresentMode;
		}
	}

	// for (const auto &availablePresentMode : availablePresentModes) {
	//   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
	//     ENGINE_INFO("Present mode: Immediate");
	//     return availablePresentMode;
	//   }
	// }

	ENGINE_INFO("Present mode: V-Sync");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
	if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actualExtent = _windowExtent;
		actualExtent.width =
			std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height =
			std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkResult Swapchain::AcquireNextImage(u32* imageIndex) {
	vkWaitForFences(
		Context::Instance->GetLogicalDevice(), 1, &_renderFences[_currentFrame], VK_TRUE, std::numeric_limits<u64>::max()
	);

	VkResult result = vkAcquireNextImageKHR(
		Context::Instance->GetLogicalDevice(),
		_swapchain,
		std::numeric_limits<u64>::max(),
		_availableSemaphores[_currentFrame],
		VK_NULL_HANDLE,
		imageIndex
	);

	return result;
}

VkResult Swapchain::SubmitCommandBuffers(const VkCommandBuffer* buffers, u32* imageIndex) {
	if (_imagesFences[*imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(Context::Instance->GetLogicalDevice(), 1, &_imagesFences[*imageIndex], VK_TRUE, UINT64_MAX);
	}
	_imagesFences[*imageIndex] = _renderFences[_currentFrame];

	vkResetFences(Context::Instance->GetLogicalDevice(), 1, &_renderFences[_currentFrame]);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[]      = {_availableSemaphores[_currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount     = 1;
	submitInfo.pWaitSemaphores        = waitSemaphores;
	submitInfo.pWaitDstStageMask      = waitStages;
	submitInfo.commandBufferCount     = 1;
	submitInfo.pCommandBuffers        = buffers;

	VkSemaphore signalSemaphores[]  = {_finishedSemaphores[_currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores    = signalSemaphores;

	VkResult result = vkQueueSubmit(Context::Instance->GetGraphicsQueue(), 1, &submitInfo, _renderFences[_currentFrame]);
	VK_CHECK(result, "failed to submit draw command buffer!");

	VkPresentInfoKHR presentInfo   = {};
	presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores    = signalSemaphores;
	VkSwapchainKHR swapChains[]    = {_swapchain};
	presentInfo.swapchainCount     = 1;
	presentInfo.pSwapchains        = swapChains;
	presentInfo.pImageIndices      = imageIndex;

	// Present image
	result = vkQueuePresentKHR(Context::Instance->GetPresentQueue(), &presentInfo);

	_currentFrame = (_currentFrame + 1) % MAX_FRAMES_SYNC;

	return result;
}

bool Swapchain::CompareFormats(const Swapchain& swapChain) const {
	bool imageFormatEqual = (swapChain._swapchainImageFormat == _swapchainImageFormat);
	return (imageFormatEqual);
}

void Swapchain::TransitImageLayout(
	VkImageLayout orgLayout, VkImageLayout trgLayout, u32 currentImageIndex, VkCommandBuffer commandBuffer
) {
	VkImageMemoryBarrier imageMemoryBarrier            = {};
	imageMemoryBarrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout                       = orgLayout;
	imageMemoryBarrier.newLayout                       = trgLayout;
	imageMemoryBarrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image                           = _swapchainImages[currentImageIndex];
	imageMemoryBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel   = 0;
	imageMemoryBarrier.subresourceRange.levelCount     = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount     = 1;

	VkPipelineStageFlags srcStage{};
	VkPipelineStageFlags dstStage{};

	if (orgLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && trgLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (orgLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && trgLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	} else {
		ENGINE_ERROR("Unsupported Layout Transition in Swap Chain!");
		return;
	}

	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}
}  // namespace VK