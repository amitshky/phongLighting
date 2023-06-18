#include "renderer/texture.h"

#include "stb_image/stb_image.h"
#include "core/core.h"
#include "utils/utils.h"
#include "renderer/device.h"
#include "renderer/commandPool.h"


Texture2D::Texture2D(const char* texturePath)
{
	CreateTextureImage(texturePath);
	CreateTextureImageView();
	CreateTextureSampler();

	m_ImageInfo.sampler = m_TextureSampler;
	m_ImageInfo.imageView = m_TextureImageView;
	m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

Texture2D::~Texture2D()
{
	vkDestroySampler(Device::GetDevice(), m_TextureSampler, nullptr);
	vkDestroyImageView(Device::GetDevice(), m_TextureImageView, nullptr);
	vkFreeMemory(Device::GetDevice(), m_TextureImageMemory, nullptr);
	vkDestroyImage(Device::GetDevice(), m_TextureImage, nullptr);
}

std::vector<VkDescriptorImageInfo> Texture2D::GetImageInfos(const std::vector<Texture2D>& textures)
{
	std::vector<VkDescriptorImageInfo> imageInfos{};
	imageInfos.reserve(textures.size());

	for (const auto& texture : textures)
		imageInfos.push_back(texture.m_ImageInfo);

	return imageInfos;
}


void Texture2D::CreateTextureImage(const char* texturePath)
{
	int width = 0, height = 0, channels = 0;
	auto imageData = stbi_load(texturePath, &width, &height, &channels, STBI_rgb_alpha);
	THROW(!imageData, "Failed to load texutre image data!")

	VkDeviceSize size = width * height * 4;
	m_Miplevels = static_cast<uint32_t>(std::log2(std::max(width, height))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	utils::CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMem);

	void* data;
	vkMapMemory(Device::GetDevice(), stagingBufferMem, 0, size, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(size));
	vkUnmapMemory(Device::GetDevice(), stagingBufferMem);

	stbi_image_free(imageData);

	// we generate mipmaps by blitting the image,
	// this operation is a transfer operation
	// so we use this image both as a dst and src
	utils::CreateImage(static_cast<uint32_t>(width),
		static_cast<uint32_t>(height),
		m_Miplevels,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | // to generate mipmaps
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_TextureImage,
		m_TextureImageMemory);

	// we wait for the queue to be idle after every operation (transition and copy)
	// this makes the operations synchronous
	// TODO: make it asynchronous

	utils::TransitionImageLayout(m_TextureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		m_Miplevels);

	utils::CopyBufferToImage(
		stagingBuffer, m_TextureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	utils::GenerateMipmaps(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, width, height, m_Miplevels);

	vkFreeMemory(Device::GetDevice(), stagingBufferMem, nullptr);
	vkDestroyBuffer(Device::GetDevice(), stagingBuffer, nullptr);
}

void Texture2D::CreateTextureImageView()
{
	m_TextureImageView =
		utils::CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_Miplevels);
}

void Texture2D::CreateTextureSampler()
{
	VkPhysicalDeviceProperties devProp = Device::GetDeviceProperties();

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = devProp.limits.maxSamplerAnisotropy;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(m_Miplevels);
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	THROW(vkCreateSampler(Device::GetDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS,
		"Failed to create texture sampler!")
}
