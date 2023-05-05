#include "utils/utils.h"

#include <stdexcept>


namespace utils {

uint32_t FindMemoryType(VkPhysicalDevice physicalDevice,
	uint32_t typeFilter,
	VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		// typeFilter specifies a bit field of memory types
		if (typeFilter & (1 << i)
			&& (memProperties.memoryTypes[i].propertyFlags & properties)
				   == properties)
			return i;
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice,
	VkSurfaceKHR windowSurface)
{
	// Simply checking swapchain availability is not enough,
	// we need to check if it is supported by our window surface or not
	// We need to check for:
	// * basic surface capabilities (min/max number of images in swap chain)
	// * surface formats (pixel format and color space)
	// * available presentation modes
	SwapchainSupportDetails swapchainDetails{};

	// query surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physicalDevice, windowSurface, &swapchainDetails.capabilities);

	// query surface format
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		physicalDevice, windowSurface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		swapchainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
			windowSurface,
			&formatCount,
			swapchainDetails.formats.data());
	}

	// query supported presentation modes
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		physicalDevice, windowSurface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		swapchainDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
			windowSurface,
			&presentModeCount,
			swapchainDetails.presentModes.data());
	}

	return swapchainDetails;
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice,
	VkSurfaceKHR windowSurface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies{ queueFamilyCount };
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice, &queueFamilyCount, queueFamilies.data());

	// find a queue that supports graphics commands
	for (int i = 0; i < queueFamilies.size(); ++i)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		// check for queue family compatible for presentation
		// the graphics queue and the presentation queue might end up being the
		// same but we treat them as separate queues
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			physicalDevice, i, windowSurface, &presentSupport);

		if (presentSupport)
			indices.presentFamily = i;

		if (indices.IsComplete())
			break;
	}

	return indices;
}

} // namespace utils