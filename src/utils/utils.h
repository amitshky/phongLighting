#pragma once

#include <vulkan/vulkan.h>


namespace utils {

void CreateImage(uint32_t width,
	uint32_t height,
	uint32_t miplevels,
	VkSampleCountFlagBits numSamples,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkImage& image,
	VkDeviceMemory& imageMemory);

VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t miplevels);

} // namespace utils