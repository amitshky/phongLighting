#include "utils/utils.h"

#include "core/core.h"
#include "renderer/device.h"

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
	VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imgCreateInfo{};
	imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imgCreateInfo.extent.width = width;
	imgCreateInfo.extent.height = height;
	imgCreateInfo.extent.depth = 1;
	imgCreateInfo.mipLevels = miplevels;
	imgCreateInfo.arrayLayers = 1;
	imgCreateInfo.format = format;
	imgCreateInfo.tiling = tiling;
	imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgCreateInfo.usage = usage;
	imgCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imgCreateInfo.samples = numSamples;
	imgCreateInfo.flags = 0; // for sparse images

	THROW(vkCreateImage(Device::GetDevice(), &imgCreateInfo, nullptr, &image) != VK_SUCCESS,
		"Failed to create image object!")

	VkMemoryRequirements memRequirements{};
	vkGetImageMemoryRequirements(Device::GetDevice(), image, &memRequirements);

	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memRequirements.size;
	memAllocInfo.memoryTypeIndex =
		Device::FindMemoryType(Device::GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

	THROW(vkAllocateMemory(Device::GetDevice(), &memAllocInfo, nullptr, &imageMemory) != VK_SUCCESS,
		"Failed to allocate image memory!")

	vkBindImageMemory(Device::GetDevice(), image, imageMemory, 0);
}

VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t miplevels)
{
	VkImageViewCreateInfo imgViewInfo{};
	imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewInfo.image = image;
	imgViewInfo.format = format;
	imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imgViewInfo.subresourceRange.aspectMask = aspectFlags;
	imgViewInfo.subresourceRange.baseMipLevel = 0;
	imgViewInfo.subresourceRange.levelCount = miplevels;
	imgViewInfo.subresourceRange.baseArrayLayer = 0;
	imgViewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	THROW(vkCreateImageView(Device::GetDevice(), &imgViewInfo, nullptr, &imageView) != VK_SUCCESS,
		"Failed to create image view!")

	return imageView;
}

} // namespace utils