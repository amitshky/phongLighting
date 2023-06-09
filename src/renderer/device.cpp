#include "renderer/device.h"

#include <set>
#include "core/core.h"
#include "renderer/vulkanContext.h"


std::shared_ptr<Device> Device::s_Instance = nullptr;

Device::Device(const VulkanConfig& config, VkSurfaceKHR windowSurface)
	: m_Config{ config },
	  m_WindowSurface{ windowSurface },
	  m_PhysicalDevice{ VK_NULL_HANDLE }
{
	PickPhysicalDevice();
	CreateLogicalDevice();
}

Device::~Device()
{
	vkDestroyDevice(m_DeviceVk, nullptr);
}

std::shared_ptr<Device> Device::Create(const VulkanConfig& config, VkSurfaceKHR windowSurface)
{
	if (s_Instance == nullptr)
	{
		s_Instance = std::make_shared<Device>(config, windowSurface);
		return s_Instance;
	}

	return s_Instance;
}

void Device::PickPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(VulkanContext::GetInstance(),
		&physicalDeviceCount,
		nullptr); // get physical device count

	THROW(physicalDeviceCount == 0, "Failed to find GPUs with Vulkan support!")

	std::vector<VkPhysicalDevice> physicalDevices{ physicalDeviceCount };
	vkEnumeratePhysicalDevices(VulkanContext::GetInstance(), &physicalDeviceCount, physicalDevices.data());

	for (const auto& device : physicalDevices)
	{
		if (IsDeviceSuitable(device))
		{
			m_PhysicalDevice = device;
			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);
			m_MsaaSamples = GetMaxUsableSampleCount();
			break;
		}
	}

	THROW(m_PhysicalDevice == VK_NULL_HANDLE, "Failed to find a suitable GPU!")

	Logger::Info(
		"Physical device info:\n"
		"    Device name: {}",
		m_PhysicalDeviceProperties.deviceName);
}

void Device::CreateLogicalDevice()
{
	// create queue
	m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, m_WindowSurface);

	// we have multiple queues so we create a set of unique queue families
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	std::set<uint32_t> uniqueQueueFamilies = { m_QueueFamilyIndices.graphicsFamily.value(),
		m_QueueFamilyIndices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (const auto& queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueFamily;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueInfo);
	}

	// specify used device features
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading

	// create logical device
	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceInfo.pEnabledFeatures = &deviceFeatures;

	// these are similar to create instance but they are device specific this
	// time
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(m_Config.deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = m_Config.deviceExtensions.data();

	if (m_Config.enableValidationLayers)
	{
		deviceInfo.enabledLayerCount = static_cast<uint32_t>(m_Config.validationLayers.size());
		deviceInfo.ppEnabledLayerNames = m_Config.validationLayers.data();
	}
	else
	{
		deviceInfo.enabledLayerCount = 0;
	}

	THROW(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_DeviceVk) != VK_SUCCESS,
		"Failed to create logcial device!");

	// get the queue handle
	vkGetDeviceQueue(m_DeviceVk, m_QueueFamilyIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_DeviceVk, m_QueueFamilyIndices.presentFamily.value(), 0, &m_PresentQueue);
}

bool Device::IsDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indicies = FindQueueFamilies(physicalDevice, m_WindowSurface);

	// checking for extension availability like swapchain extension availability
	bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice);

	// checking if swapchain is supported by window surface
	bool swapchainAdequate = false;
	if (extensionsSupported)
	{
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(physicalDevice, m_WindowSurface);
		swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

	return indicies.IsComplete() && extensionsSupported && swapchainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions{ extensionCount };
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions{ m_Config.deviceExtensions.begin(), m_Config.deviceExtensions.end() };

	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

VkSampleCountFlagBits Device::GetMaxUsableSampleCount()
{
	VkSampleCountFlags counts = m_PhysicalDeviceProperties.limits.framebufferColorSampleCounts
								& m_PhysicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return VK_SAMPLE_COUNT_64_BIT;

	if (counts & VK_SAMPLE_COUNT_32_BIT)
		return VK_SAMPLE_COUNT_32_BIT;

	if (counts & VK_SAMPLE_COUNT_16_BIT)
		return VK_SAMPLE_COUNT_16_BIT;

	if (counts & VK_SAMPLE_COUNT_8_BIT)
		return VK_SAMPLE_COUNT_8_BIT;

	if (counts & VK_SAMPLE_COUNT_4_BIT)
		return VK_SAMPLE_COUNT_4_BIT;

	if (counts & VK_SAMPLE_COUNT_2_BIT)
		return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}


uint32_t Device::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		// typeFilter specifies a bit field of memory types
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	THROW(true, "Failed to find suitable memory type!");
}

SwapchainSupportDetails Device::QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface)
{
	// Simply checking swapchain availability is not enough,
	// we need to check if it is supported by our window surface or not
	// We need to check for:
	// * basic surface capabilities (min/max number of images in swap chain)
	// * surface formats (pixel format and color space)
	// * available presentation modes
	SwapchainSupportDetails swapchainDetails{};

	// query surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, windowSurface, &swapchainDetails.capabilities);

	// query surface format
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, windowSurface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		swapchainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physicalDevice, windowSurface, &formatCount, swapchainDetails.formats.data());
	}

	// query supported presentation modes
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, windowSurface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		swapchainDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice, windowSurface, &presentModeCount, swapchainDetails.presentModes.data());
	}

	return swapchainDetails;
}

QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies{ queueFamilyCount };
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	// find a queue that supports graphics commands
	for (int i = 0; i < queueFamilies.size(); ++i)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		// check for queue family compatible for presentation
		// the graphics queue and the presentation queue might end up being the
		// same but we treat them as separate queues
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, windowSurface, &presentSupport);

		if (presentSupport)
			indices.presentFamily = i;

		if (indices.IsComplete())
			break;
	}

	return indices;
}

VkFormat Device::FindSupportedFormat(const std::vector<VkFormat>& canditateFormats,
	VkImageTiling tiling,
	VkFormatFeatureFlags features)
{
	for (auto format : canditateFormats)
	{
		VkFormatProperties prop;
		vkGetPhysicalDeviceFormatProperties(s_Instance->GetPhysicalDevice(), format, &prop);

		if (tiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (prop.optimalTilingFeatures & features) == features)
			return format;
	}

	THROW(true, "Failed to find supported format!");
}