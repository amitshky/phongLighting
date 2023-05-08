#pragma once

#include <vulkan/vulkan.h>


namespace utils {

VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t miplevels);

}