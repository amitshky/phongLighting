#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <initializer_list>
#include "renderer/shader.h"
#include "renderer/uniformBuffer.h"
#include "renderer/texture.h"


enum class DescriptorType
{
	COMBINED_IMAGE_SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	STORAGE_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	UNIFORM_BUFFER_DYNAMIC = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	STORAGE_BUFFER_DYNAMIC = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
};

struct DescriptorLayout
{
	uint32_t shaderBinding;
	DescriptorType descriptorType;
	uint32_t bindingDescriptorCount;
	ShaderType shaderStageFlags;

	UniformBuffer* pUniformBuffer = nullptr;
	uint32_t uniformBufferCount;
	Texture2D* pTexture = nullptr;
	uint32_t textureCount;
};

class DescriptorPool
{
public:
	DescriptorPool();
	~DescriptorPool();

private:
	void CreateDescriptorPool();

private:
	VkDescriptorPool m_DescriptorPool;
};

class DescriptorSet
{
public:
	DescriptorSet(uint32_t descriptorSetCount);
	~DescriptorSet();

	void Init(uint32_t descriptorSetCount);

	void SetupLayout(std::initializer_list<DescriptorLayout> layout);
	void Create();

	static DescriptorLayout CreateLayout(DescriptorType descriptorType,
		ShaderType shaderStage,
		uint32_t shaderBinding,
		uint32_t bindingDescriptorCount,
		UniformBuffer* pUniformBuffer,
		uint32_t uniformBufferCount,
		Texture2D* pTexture,
		uint32_t textureCount);

	inline VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

	void Bind(VkCommandBuffer commandBuffer,
		uint64_t currentFrameIdx,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets);

private:
	void CreateDescriptorPool();
	void Cleanup();

private:
	uint32_t m_DescriptorSetCount = 0;
	std::vector<DescriptorLayout> m_DescriptorLayout{};
	std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings{};
	VkDescriptorSetLayout m_DescriptorSetLayout{};
	VkPipelineLayout m_PipelineLayout{};
	VkDescriptorPool m_DescriptorPool{};
	std::vector<VkDescriptorSet> m_DescriptorSets{};
};