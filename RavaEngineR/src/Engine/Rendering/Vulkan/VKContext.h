#pragma once

namespace VK {
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

struct QueueFamilyIndices {
	u32 GraphicsFamily;
	u32 PresentFamily;
	bool GraphicsFamilyHasValue = false;
	bool PresentFamilyHasValue  = false;
	bool IsComplete() const { return GraphicsFamilyHasValue && PresentFamilyHasValue; }
};

class Window;
class Context {
   public:
	static Unique<Context> Instance;

   public:
	Context(Window* window);
	~Context();

	NO_COPY(Context)
	NO_MOVE(Context)

	VkInstance GetInstance() const { return _instance; }
	VkSurfaceKHR GetSurface() const { return _surface; }
	VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
	VkDevice GetLogicalDevice() const { return _device; }
	VkCommandPool GetCommandPool() const { return _commandPool; }
	VkQueue GetGraphicsQueue() const { return _graphicsQueue; }
	VkQueue GetPresentQueue() const { return _presentQueue; }
	QueueFamilyIndices& GetPhysicalQueueFamilies() { return _queueFamilyIndices; }
	SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(_physicalDevice); }
	const VkPhysicalDeviceProperties& GetProperties() { return _properties; }

   private:
	Window* _window;
	VkInstance _instance;
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	VkDevice _device;
	VkSurfaceKHR _surface;
	VkCommandPool _commandPool;
	VkPhysicalDeviceProperties _properties;

	QueueFamilyIndices _queueFamilyIndices;
	VkQueue _graphicsQueue;
	VkQueue _presentQueue;

	VkDebugUtilsMessengerEXT _debugMessenger;

   private:
	void CreateInstance();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateCommandPool();

	void SetupDebugMessenger();
	bool CheckValidationLayerSupport();

	std::vector<const char*> GetRequiredExtensions();
	void HasGflwRequiredInstanceExtensions();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
};
}  // namespace VK