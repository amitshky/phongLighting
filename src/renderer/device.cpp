#include "renderer/device.h"

#include <set>
#include <stdexcept>
#include "core/logger.h"
#include "utils/utils.h"


Device::Device(const VulkanConfig& config,
	VkInstance vulkanInstance,
	VkSurfaceKHR windowSurface)
	: m_Config{ config },
	  m_VulkanInstance{ vulkanInstance },
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

void Device::PickPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_VulkanInstance,
		&physicalDeviceCount,
		nullptr); // get physical device count

	if (physicalDeviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> physicalDevices{ physicalDeviceCount };
	vkEnumeratePhysicalDevices(
		m_VulkanInstance, &physicalDeviceCount, physicalDevices.data());

	for (const auto& device : physicalDevices)
	{
		if (IsDeviceSuitable(device))
		{
			m_PhysicalDevice = device;
			m_MsaaSamples = GetMaxUsableSampleCount();
			break;
		}
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a suitable GPU!");

	// we dont need to write this
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

	Logger::Info(
		"Physical device info:\n"
		"    Device name: {}\n",
		physicalDeviceProperties.deviceName);
}

void Device::CreateLogicalDevice()
{
	// create queue
	utils::QueueFamilyIndices indices =
		utils::FindQueueFamilies(m_PhysicalDevice, m_WindowSurface);

	// we have multiple queues so we create a set of unique queue families
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (const auto& queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// specify used device features
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading

	// create logical device
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount =
		static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// these are similar to create instance but they are device specific this
	// time
	deviceCreateInfo.enabledExtensionCount =
		static_cast<uint32_t>(m_Config.deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = m_Config.deviceExtensions.data();

	if (m_Config.enableValidationLayers)
	{
		deviceCreateInfo.enabledLayerCount =
			static_cast<uint32_t>(m_Config.validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = m_Config.validationLayers.data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(
			m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_DeviceVk)
		!= VK_SUCCESS)
		throw std::runtime_error("Failed to create logcial device!");

	// get the queue handle
	vkGetDeviceQueue(
		m_DeviceVk, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(
		m_DeviceVk, indices.presentFamily.value(), 0, &m_PresentQueue);
}

bool Device::IsDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	utils::QueueFamilyIndices indicies =
		utils::FindQueueFamilies(physicalDevice, m_WindowSurface);

	// checking for extension availability like swapchain extension availability
	bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice);

	// checking if swapchain is supported by window surface
	bool swapchainAdequate = false;
	if (extensionsSupported)
	{
		utils::SwapchainSupportDetails swapchainSupport =
			utils::QuerySwapchainSupport(physicalDevice, m_WindowSurface);
		swapchainAdequate = !swapchainSupport.formats.empty()
							&& !swapchainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

	return indicies.IsComplete() && extensionsSupported && swapchainAdequate
		   && supportedFeatures.samplerAnisotropy;
}

bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(
		physicalDevice, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions{ extensionCount };
	vkEnumerateDeviceExtensionProperties(
		physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions{ m_Config.deviceExtensions.begin(),
		m_Config.deviceExtensions.end() };

	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

VkSampleCountFlagBits Device::GetMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts =
		physicalDeviceProperties.limits.framebufferColorSampleCounts
		& physicalDeviceProperties.limits.framebufferDepthSampleCounts;

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