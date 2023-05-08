#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "core/window.h"


struct VulkanConfig
{
public:
	bool enableValidationLayers;
	int maxFramesInFlight;

	std::vector<const char*> validationLayers;
	std::vector<const char*> deviceExtensions;

public:
	VulkanConfig(bool enableValidationLayers,
		int maxFramesInFlight,
		const std::vector<const char*>& validationLayers,
		const std::vector<const char*>& deviceExtensions)
		: enableValidationLayers{ enableValidationLayers },
		  maxFramesInFlight{ maxFramesInFlight },
		  validationLayers{ validationLayers },
		  deviceExtensions{ deviceExtensions }
	{}
};

class VulkanContext
{
public:
	VulkanContext(const VulkanContext&) = delete;
	VulkanContext& operator=(const VulkanContext&) = delete;
	~VulkanContext();

	static VulkanContext* Create(const char* title, const VulkanConfig& config, const std::shared_ptr<Window>& window);
	static inline VkInstance GetInstance() { return s_Instance->m_VulkanInstance; }
	static inline VkSurfaceKHR GetWindowSurface() { return s_Instance->m_Window->GetWindowSurface(); }

private:
	VulkanContext(const char* title, const VulkanConfig& config, const std::shared_ptr<Window>& window);

	void CreateInstance(const char* title);
	std::vector<const char*> GetRequiredExtensions();

	void SetupDebugMessenger();
	bool CheckValidationLayerSupport();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo);

	// `VKAPI_ATTR` and `VKAPI_CALL` ensures the right signature for Vulkan
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbakck,
		void* pUserData);

	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);

private:
	const VulkanConfig m_Config;
	std::shared_ptr<Window> m_Window;

	static VulkanContext* s_Instance;
	VkInstance m_VulkanInstance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
};