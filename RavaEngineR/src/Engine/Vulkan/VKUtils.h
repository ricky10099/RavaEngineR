#pragma once

// #include "Framework/Vulkan/GPUSharedDefines.h"
#include "Engine/Vulkan/VKContext.h"

//////////////////////////////////////////////////////////////////////////
// Vulkan config
//////////////////////////////////////////////////////////////////////////
static constexpr int MAX_FRAMES_SYNC = 2;

const std::vector<const char*> DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME};

//////////////////////////////////////////////////////////////////////////
// Vulkan validation
//////////////////////////////////////////////////////////////////////////
#define VK_CHECK(x, msg)      \
	if (x != VK_SUCCESS) {    \
		ENGINE_CRITICAL(msg); \
	}

static void CheckVKResult(VkResult err) {
	std::string_view msg("[vulkan] Error: VkResult = {0}", err);
	VK_CHECK(err, msg);
}

#ifdef RV_DEBUG
static const bool ENABLE_VALIDATION = true;
#else
static const bool ENABLE_VALIDATION = false;
#endif

// VK_LAYER_LUNARG_standard_validation = All standard validation layers
static const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

namespace VK {
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
) {
	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			ENGINE_ERROR(pCallbackData->pMessage);
			return VK_TRUE;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			ENGINE_WARN(pCallbackData->pMessage);
			return VK_FALSE;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			ENGINE_INFO(pCallbackData->pMessage);
			return VK_FALSE;
		//case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		//	ENGINE_TRACE(pCallbackData->pMessage);
		//	return VK_FALSE;
		default:
			return VK_FALSE;
	}
}

static VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugUtilsMessengerEXT(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator
) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo                 = {};
	createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
							   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
						   | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
}
}  // namespace VK

//////////////////////////////////////////////////////////////////////////
// Vulkan Function
//////////////////////////////////////////////////////////////////////////
namespace VK {
static VkFormat FindSupportedFormat(
	const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features
) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(Context::Instance->GetPhysicalDevice(), format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	ENGINE_CRITICAL("Failed to Find Supported Format!");
	return {};
}

static VkFormat FindDepthFormat() {
	return FindSupportedFormat(
		{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

static u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(Context::Instance->GetPhysicalDevice(), &memProperties);
	for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i))
			&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	ENGINE_CRITICAL("Failed to Find Suitable Memory Type!");
	return 0;
}

static void CreateBuffer(
	VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory
) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size        = size;
	bufferInfo.usage       = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(Context::Instance->GetLogicalDevice(), &bufferInfo, nullptr, &buffer);
	VK_CHECK(result, "Failed to Create Vertex Buffer!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(Context::Instance->GetLogicalDevice(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	result = vkAllocateMemory(Context::Instance->GetLogicalDevice(), &allocInfo, nullptr, &bufferMemory);
	VK_CHECK(result, "Failed to Allocate Vertex Buffer Memory!");

	vkBindBufferMemory(Context::Instance->GetLogicalDevice(), buffer, bufferMemory, 0);
}

static VkCommandBuffer BeginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool        = Context::Instance->GetCommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(Context::Instance->GetLogicalDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

static void EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &commandBuffer;

	vkQueueSubmit(Context::Instance->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(Context::Instance->GetGraphicsQueue());
	vkFreeCommandBuffers(Context::Instance->GetLogicalDevice(), Context::Instance->GetCommandPool(), 1, &commandBuffer);
}

static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size      = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

static void CopyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height, u32 layerCount) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset      = 0;
	region.bufferRowLength   = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel       = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount     = layerCount;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	EndSingleTimeCommands(commandBuffer);
}

static void TransitionImageLayout(VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, u32 mipLevels = 1) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier imageMemoryBarrier            = {};
	imageMemoryBarrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout                       = oldLayout;
	imageMemoryBarrier.newLayout                       = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image                           = image;
	imageMemoryBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel   = 0;
	imageMemoryBarrier.subresourceRange.levelCount     = mipLevels;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount     = 1;

	VkPipelineStageFlags srcStage{};
	VkPipelineStageFlags dstStage{};

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	} else {
		ENGINE_ERROR("Unsupported Layout Transition!");
		return;
	}

	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	EndSingleTimeCommands(commandBuffer);
}

static void CreateImage(
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags useFlags,
	VkMemoryPropertyFlags propFlags,
	VkDeviceMemory& imageMemory,
	VkImage& image,
	u32 mipLevels = 1
) {
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType         = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width      = width;
	imageCreateInfo.extent.height     = height;
	imageCreateInfo.extent.depth      = 1;
	imageCreateInfo.mipLevels         = mipLevels;
	imageCreateInfo.arrayLayers       = 1;
	imageCreateInfo.format            = format;
	imageCreateInfo.tiling            = tiling;
	imageCreateInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage             = useFlags;
	imageCreateInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateImage(Context::Instance->GetLogicalDevice(), &imageCreateInfo, nullptr, &image);
	VK_CHECK(result, "Failed to create an Image!");

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(Context::Instance->GetLogicalDevice(), image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize       = memoryRequirements.size;
	memoryAllocInfo.memoryTypeIndex      = FindMemoryType(memoryRequirements.memoryTypeBits, propFlags);

	result = vkAllocateMemory(Context::Instance->GetLogicalDevice(), &memoryAllocInfo, nullptr, &imageMemory);
	VK_CHECK(result, "Failed to allocate memory for image!");

	vkBindImageMemory(Context::Instance->GetLogicalDevice(), image, imageMemory, 0);
}

static void CreateImageView(
	VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView, u32 mipLevels = 1
) {
	VkImageViewCreateInfo viewCreateInfo           = {};
	viewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image                           = image;
	viewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format                          = format;
	viewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.subresourceRange.aspectMask     = aspectFlags;
	viewCreateInfo.subresourceRange.baseMipLevel   = 0;
	viewCreateInfo.subresourceRange.levelCount     = mipLevels;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount     = 1;

	VkResult result = vkCreateImageView(Context::Instance->GetLogicalDevice(), &viewCreateInfo, nullptr, &imageView);
	VK_CHECK(result, "Failed to create an Image View!");
}

static std::vector<char> ReadShaderFromAssets(const std::string& filename) {
	// Open stream from given file
	// std::ios::binary tells stream to read file as binary
	// std::ios::ate tells stream to start reading from the and of file
	std::string assetsPath = ASSETS_DIR + filename;

	std::ifstream file(assetsPath, std::ios::binary | std::ios::ate);

	if (!file.is_open()) {
		ENGINE_ERROR("Failed to open a file: " + assetsPath);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> fileBuffer(fileSize);

	file.seekg(0);
	file.read(fileBuffer.data(), fileSize);
	file.close();

	return fileBuffer;
}
}  // namespace VK