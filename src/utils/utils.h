#pragma once

#include <vector>
#include <optional>
#include <vulkan/vulkan.h>

namespace utils {

struct QueueFamilyIndices
{
	// std::optional contains no value until you assign something to it
	// we can check if it contains a value by calling has_value()
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	inline bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

uint32_t FindMemoryType(VkPhysicalDevice physicalDevice,
	uint32_t typeFilter,
	VkMemoryPropertyFlags properties);

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice,
	VkSurfaceKHR windowSurface);

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice,
	VkSurfaceKHR windowSurface);

} // namespace utils