#include "rvpch.h"

#include "Engine/Vulkan/VKContext.h"
#include "Engine/Vulkan/VKUtils.h"
#include "Engine/Vulkan/VKWindow.h"

namespace VK {
Unique<Context> Context::Instance = nullptr;

Context::Context(Window* window)
	: _window(window) {

	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateCommandPool();
}

Context::~Context() {
	vkDeviceWaitIdle(_device);
	vkDestroyCommandPool(_device, _commandPool, nullptr);
	vkDestroyDevice(_device, nullptr);

	if (ENABLE_VALIDATION) {
		DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(_instance, _surface, nullptr);
	vkDestroyInstance(_instance, nullptr);
}

void Context::CreateInstance() {
	if (ENABLE_VALIDATION && !CheckValidationLayerSupport()) {
		ENGINE_CRITICAL("Validation Layers requested, but not available!");
	}

	VkApplicationInfo appInfo  = {};
	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "Rava Engine R";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName        = "Rava Engine";
	appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_4;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo     = &appInfo;

	auto extensions = GetRequiredExtensions();
	HasGflwRequiredInstanceExtensions();

	createInfo.enabledExtensionCount   = static_cast<u32>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (ENABLE_VALIDATION) {
		createInfo.enabledLayerCount   = static_cast<u32>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext             = nullptr;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
	VK_CHECK(result, "Failed to Create a Vulkan Instance!")
}

void Context::CreateSurface() {
	VkResult result = glfwCreateWindowSurface(_instance, (GLFWwindow*)_window->GetBackendWindow(), nullptr, &_surface);
	VK_CHECK(result, "Failed to create a surface!");
}

void Context::PickPhysicalDevice() {
	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		ENGINE_CRITICAL("Failed to Find GPUs with Vulkan support!");
	}
	ENGINE_INFO("Device count: {0}", deviceCount);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (IsDeviceSuitable(device)) {
			_physicalDevice = device;
			break;
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE) {
		ENGINE_CRITICAL("Failed to Find a suitable GPU!");
	}

	vkGetPhysicalDeviceProperties(_physicalDevice, &_properties);
	ENGINE_INFO("Physical Device: ", _properties.deviceName);
}

void Context::CreateLogicalDevice() {
	_queueFamilyIndices = FindQueueFamilies(_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<u32> uniqueQueueFamilies = {_queueFamilyIndices.GraphicsFamily, _queueFamilyIndices.PresentFamily};

	float queuePriority = 1.0f;
	for (u32 queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex        = queueFamily;
		queueCreateInfo.queueCount              = 1;
		queueCreateInfo.pQueuePriorities        = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy        = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos    = queueCreateInfos.data();

	createInfo.pEnabledFeatures        = &deviceFeatures;
	createInfo.enabledExtensionCount   = static_cast<u32>(DEVICE_EXTENSIONS.size());
	createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

	VkResult result = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device);
	VK_CHECK(result, "Failed to Create a Logical Device!")

	vkGetDeviceQueue(_device, _queueFamilyIndices.GraphicsFamily, 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, _queueFamilyIndices.PresentFamily, 0, &_presentQueue);
}

void Context::CreateCommandPool() {
	QueueFamilyIndices queueFamilyIndices = _queueFamilyIndices;

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags                   = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex        = queueFamilyIndices.GraphicsFamily;

	VkResult result = vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool);
	VK_CHECK(result, "Failed to Create Command Pool!");
}

void Context::SetupDebugMessenger() {
	if (!ENABLE_VALIDATION) {
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);
	VkResult result = CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger);
	VK_CHECK(result, "Failed to Set Up Debug Messenger!")
}

bool Context::CheckValidationLayerSupport() {
	u32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : VALIDATION_LAYERS) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> Context::GetRequiredExtensions() {
	u32 glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (ENABLE_VALIDATION) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void Context::HasGflwRequiredInstanceExtensions() {
	u32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	ENGINE_INFO("Available Extensions:");
	std::unordered_set<std::string_view> available;
	for (const auto& extension : extensions) {
		ENGINE_INFO("\t{0}", extension.extensionName);
		available.insert(extension.extensionName);
	}

	ENGINE_INFO("Required Extensions:");
	auto requiredExtensions = GetRequiredExtensions();
	for (const auto& required : requiredExtensions) {
		ENGINE_INFO("\t{0}", required);
		if (available.find(required) == available.end()) {
			ENGINE_CRITICAL("Missing required glfw extension");
		}
	}
}

bool Context::IsDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate                        = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices Context::FindQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.GraphicsFamily         = i;
			indices.GraphicsFamilyHasValue = true;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.PresentFamily         = i;
			indices.PresentFamilyHasValue = true;
		}

		if (indices.IsComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

bool Context::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
	u32 extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	if (extensionCount == 0) {
		return false;
	}

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string_view> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails Context::QuerySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.Capabilities);

	u32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.Formats.data());
	}

	u32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.PresentModes.data());
	}
	return details;
}
}  // namespace VK