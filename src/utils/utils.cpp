#include "utils/utils.h"

#include "core/core.h"
#include "renderer/device.h"

namespace utils {

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