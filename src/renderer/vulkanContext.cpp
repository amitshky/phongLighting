#include "renderer/vulkanContext.h"

#include <stdexcept>
#include "core/logger.h"
#include "core/window.h"


VulkanContext::VulkanContext(const char* title,
	const VulkanConfig& config,
	const std::shared_ptr<Window>& window)
	: m_Config{ config },
	  m_Window{ window }
{
	CreateInstance(title);
	SetupDebugMessenger();
	window->CreateWindowSurface(m_VulkanInstance);
}

VulkanContext::~VulkanContext()
{
	m_Window->DestroyWindowSurface(m_VulkanInstance);

	if (m_Config.enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(
			m_VulkanInstance, m_DebugMessenger, nullptr);

	vkDestroyInstance(m_VulkanInstance, nullptr);
}

void VulkanContext::CreateInstance(const char* title)
{
	if (m_Config.enableValidationLayers && !CheckValidationLayerSupport())
		throw std::runtime_error(
			"Validation layers requested, but not available!");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = title;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0; // the highest version the
											 // application is designed to use

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	// get required extensions
	std::vector<const char*> extensions = GetRequiredExtensions();
	instanceInfo.enabledExtensionCount =
		static_cast<uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();

	// specify global validation layers
	if (m_Config.enableValidationLayers)
	{
		instanceInfo.enabledLayerCount =
			static_cast<uint32_t>(m_Config.validationLayers.size());
		instanceInfo.ppEnabledLayerNames = m_Config.validationLayers.data();

		// debug messenger
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
		PopulateDebugMessengerCreateInfo(debugMessengerInfo);
		instanceInfo.pNext =
			(VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerInfo;
	}
	else
	{
		instanceInfo.enabledLayerCount = 0;
		instanceInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&instanceInfo, nullptr, &m_VulkanInstance)
		!= VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance!");
}

std::vector<const char*> VulkanContext::GetRequiredExtensions()
{
	uint32_t extensionCount = 0;
	const char** extensions =
		Window::GetRequiredVulkanExtensions(&extensionCount);
	std::vector<const char*> availableExtensions{ extensions,
		extensions + extensionCount };

	if (m_Config.enableValidationLayers)
		availableExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return availableExtensions;
}

void VulkanContext::SetupDebugMessenger()
{
	if (!m_Config.enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
	PopulateDebugMessengerCreateInfo(debugMessengerInfo);

	if (CreateDebugUtilsMessengerEXT(
			m_VulkanInstance, &debugMessengerInfo, nullptr, &m_DebugMessenger)
		!= VK_SUCCESS)
		throw std::runtime_error("Failed to setup debug messenger!");
}

bool VulkanContext::CheckValidationLayerSupport()
{
	// check if all of the requested layers are available
	uint32_t validationLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);
	std::vector<VkLayerProperties> availableLayerProperites{
		validationLayerCount
	};
	vkEnumerateInstanceLayerProperties(
		&validationLayerCount, availableLayerProperites.data());

	for (const auto& layer : m_Config.validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayerProperites)
		{
			if (std::strcmp(layer, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

// the returned value indicates if the Vulkan call that triggered the validation
// layer message should be aborted if true, the call is aborted with
// `VK_ERROR_VALIDATION_FAILED_EXT` error
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT
		messageSeverity, // to check the severity of the message
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT*
		pCallbackData, // contains the actual details of the message
	void* pUserData) // allows you to pass your own data
{
	Logger::Error("Validation layer: {}\n", pCallbackData->pMessage);
	return VK_FALSE;
}

void VulkanContext::PopulateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo)
{
	// debug messenger provides explicit control over what kind of messages to
	// log
	debugMessengerInfo = {};
	debugMessengerInfo.sType =
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	//   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	debugMessengerInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
	debugMessengerInfo.pfnUserCallback =
		DebugCallback; // call back function for debug messenger
	debugMessengerInfo.pUserData = nullptr; // Optional
}

VkResult VulkanContext::CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	// requires a valid instance to have been created
	// so we cannot debug any issues in vkCreateInstance

	// get the function pointer for creating debug utils messenger
	// returns nullptr if the function couldn't be loaded
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void VulkanContext::DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	// must be destroyed before instance is destroyed
	// so we cannot debug any issues in vkDestroyInstance
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}