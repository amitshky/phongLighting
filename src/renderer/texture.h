#pragma once

#include <vector>
#include <vulkan/vulkan.h>


class Texture2D
{
public:
	Texture2D(const char* texturePath);
	~Texture2D();

	inline VkDescriptorImageInfo& GetImageInfo() { return m_ImageInfo; }

	static std::vector<VkDescriptorImageInfo> GetImageInfos(const std::vector<Texture2D>& textures);

private:
	void CreateTextureImage(const char* texturePath);
	void CreateTextureImageView();
	void CreateTextureSampler();

private:
	uint32_t m_Miplevels = 0;
	VkDescriptorImageInfo m_ImageInfo{};
	VkImage m_TextureImage{};
	VkDeviceMemory m_TextureImageMemory{};
	VkImageView m_TextureImageView{};
	VkSampler m_TextureSampler{};
};