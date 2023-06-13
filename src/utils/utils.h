#pragma once

#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include "renderer/vertexBuffer.h"


namespace utils {

std::pair<std::vector<uint32_t>, std::vector<Vertex>> GetModelData(const std::vector<Vertex>& vertices);

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

VkCommandBuffer BeginSingleTimeCommands();
void EndSingleTimeCommands(VkCommandBuffer cmdBuff);

void CreateBuffer(VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory);

void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

void GenerateMipmaps(VkImage image, VkFormat format, int32_t width, int32_t height, uint32_t mipLevels);

void TransitionImageLayout(VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t miplevels);

} // namespace utils