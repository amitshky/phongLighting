#include "utils/utils.h"

#include <utility>
#include "core/core.h"
#include "renderer/device.h"
#include "renderer/commandPool.h"

namespace utils {

std::pair<std::vector<uint32_t>, std::vector<Vertex>> GetModelData(const std::vector<Vertex>& vertices)
{
	std::unordered_map<Vertex, uint32_t> vertexLookup{};
	std::vector<uint32_t> indices{};
	std::vector<Vertex> uniqueVertices{};
	uint32_t i = 0;

	for (const auto& vertex : vertices)
	{
		if (vertexLookup.count(vertex) == 0)
		{
			vertexLookup[vertex] = i;
			uniqueVertices.push_back(vertex);
			++i;
		}

		indices.push_back(vertexLookup[vertex]);
	}

	return { indices, uniqueVertices };
}

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
	VkImageCreateInfo imgInfo{};
	imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imgInfo.imageType = VK_IMAGE_TYPE_2D;
	imgInfo.extent.width = width;
	imgInfo.extent.height = height;
	imgInfo.extent.depth = 1;
	imgInfo.mipLevels = miplevels;
	imgInfo.arrayLayers = 1;
	imgInfo.format = format;
	imgInfo.tiling = tiling;
	imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgInfo.usage = usage;
	imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imgInfo.samples = numSamples;
	imgInfo.flags = 0; // for sparse images

	THROW(vkCreateImage(Device::GetDevice(), &imgInfo, nullptr, &image) != VK_SUCCESS, "Failed to create image object!")

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

void CreateBuffer(VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	THROW(vkCreateBuffer(Device::GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS, "Failed to create buffer!")

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(Device::GetDevice(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocMemory{};
	allocMemory.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocMemory.allocationSize = memRequirements.size;
	allocMemory.memoryTypeIndex =
		Device::FindMemoryType(Device::GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

	THROW(vkAllocateMemory(Device::GetDevice(), &allocMemory, nullptr, &bufferMemory) != VK_SUCCESS,
		"Failed to allocate memory!")

	vkBindBufferMemory(Device::GetDevice(), buffer, bufferMemory, 0);
}

VkCommandBuffer BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo cmdBuffAllocInfo{};
	cmdBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBuffAllocInfo.commandPool = CommandPool::Get();
	cmdBuffAllocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuff;
	vkAllocateCommandBuffers(Device::GetDevice(), &cmdBuffAllocInfo, &cmdBuff);

	// immediately start recording the command buffer
	VkCommandBufferBeginInfo cmdBuffBegin{};
	cmdBuffBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBuffBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuff, &cmdBuffBegin);

	return cmdBuff;
}

void EndSingleTimeCommands(VkCommandBuffer cmdBuff)
{
	vkEndCommandBuffer(cmdBuff);

	// submit the queue
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuff;

	vkQueueSubmit(Device::GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(Device::GetGraphicsQueue());

	vkFreeCommandBuffers(Device::GetDevice(), CommandPool::Get(), 1, &cmdBuff);
}

void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer cmdBuff = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	// transfer the contents of the buffers
	vkCmdCopyBuffer(cmdBuff, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(cmdBuff);
}

void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer cmdBuff = BeginSingleTimeCommands();

	// specify which part of the buffer is going to be copied to which part of the image
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferImageHeight = 0;
	region.bufferRowLength = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	// part of the image to copy to
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(cmdBuff, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	EndSingleTimeCommands(cmdBuff);
}

void GenerateMipmaps(VkImage image, VkFormat format, int32_t width, int32_t height, uint32_t mipLevels)
{
	// TODO: load mipmaps from a file instead of generating them

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(Device::GetPhysicalDevice(), format, &formatProperties);
	// check image format's support for linear filter
	THROW(!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
		"Texture image format does not support linear blitting!");

	VkCommandBuffer cmdBuff = BeginSingleTimeCommands();

	VkImageMemoryBarrier imgBarrier{};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = image;
	imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgBarrier.subresourceRange.baseArrayLayer = 0;
	imgBarrier.subresourceRange.layerCount = 1;
	imgBarrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = width;
	int32_t mipHeight = height;

	for (uint32_t i = 1; i < mipLevels; ++i)
	{
		// transition the `i - 1` mip level to
		// `VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL`
		imgBarrier.subresourceRange.baseMipLevel = i - 1;
		imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuff,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imgBarrier);

		// specify the region to be used in blit operation
		// the src mip level is `i - 1`
		// the dst mip level is `i`
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(cmdBuff,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&blit,
			VK_FILTER_LINEAR);

		// trasition the `i - 1` mip level to
		// `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL` all the sampling
		// operations will wait on this transition to finish
		imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuff,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imgBarrier);

		// one of the dimensions will reach 1 before the other, so keep it 1
		// when it does (because the image is not a square)
		if (mipWidth > 1)
			mipWidth /= 2;

		if (mipHeight > 1)
			mipHeight /= 2;
	}

	// this barrier transitions the last mip level from
	// `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL` to
	// `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL` the loop doesnt handle
	// this
	imgBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(cmdBuff,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imgBarrier);

	EndSingleTimeCommands(cmdBuff);
}

void TransitionImageLayout(VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t miplevels)
{
	VkCommandBuffer cmdBuff = BeginSingleTimeCommands();

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = miplevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0; // operation before the barrier
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // operation after the barrier

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		LOG_AND_THROW("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(cmdBuff, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	EndSingleTimeCommands(cmdBuff);
}

} // namespace utils