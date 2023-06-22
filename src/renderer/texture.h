#pragma once

#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>


class Texture2D
{
public:
	Texture2D(const char* texturePath);
	~Texture2D();

	inline std::string GetPath() const { return m_Path; }
	inline VkDescriptorImageInfo& GetImageInfo() { return m_ImageInfo; }

	static std::vector<VkDescriptorImageInfo> GetImageInfos(const std::vector<Texture2D>& textures);
	static std::vector<VkDescriptorImageInfo> GetImageInfos(const std::vector<std::shared_ptr<Texture2D>>& textures);

private:
	void CreateTextureImage(const char* texturePath);
	void CreateTextureImageView();
	void CreateTextureSampler();

private:
	std::string m_Path;
	uint32_t m_Miplevels = 0;
	VkDescriptorImageInfo m_ImageInfo{};
	VkImage m_TextureImage{};
	VkDeviceMemory m_TextureImageMemory{};
	VkImageView m_TextureImageView{};
	VkSampler m_TextureSampler{};
};