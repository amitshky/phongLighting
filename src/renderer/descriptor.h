#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <initializer_list>
#include "renderer/shader.h"
#include "renderer/uniformBuffer.h"
#include "renderer/texture.h"


enum class DescriptorType
{
	SAMPLER = VK_DESCRIPTOR_TYPE_SAMPLER,
	COMBINED_IMAGE_SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	SAMPLED_IMAGE = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	STORAGE_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	UNIFORM_BUFFER_DYNAMIC = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	STORAGE_BUFFER_DYNAMIC = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
};

struct DescriptorLayout
{
	DescriptorType descriptorType;
	ShaderType shaderStageFlags;
	uint32_t shaderBinding;
	uint32_t descriptorCount;

	VkDescriptorBufferInfo* pBufferInfos = nullptr;
	VkDescriptorImageInfo* pImageInfos = nullptr;
};

class DescriptorPool
{
public:
	static void Init();
	static void Cleanup();
	static inline VkDescriptorPool Get() { return s_DescriptorPool; }

private:
	static VkDescriptorPool s_DescriptorPool;
};

class DescriptorSet
{
public:
	DescriptorSet(uint32_t descriptorSetCount);
	~DescriptorSet();

	static void CreateDescriptorPool();
	void Init(uint32_t descriptorSetCount);
	void Cleanup();

	void SetupLayout(std::initializer_list<DescriptorLayout> layout);
	void Create();

	static DescriptorLayout CreateLayout(DescriptorType descriptorType,
		ShaderType shaderStage,
		uint32_t shaderBinding,
		uint32_t descriptorCount,
		VkDescriptorBufferInfo* pBufferInfos,
		VkDescriptorImageInfo* pImageInfos);

	inline VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

	inline void Bind(VkCommandBuffer commandBuffer,
		uint64_t currentFrameIdx,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets)
	{
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout,
			0,
			1,
			&m_DescriptorSets[currentFrameIdx],
			dynamicOffsetCount,
			pDynamicOffsets);
	}

	inline void PushConstants(VkCommandBuffer commandBuffer, uint64_t currentFrameIdx, const void* pValues)
	{
		vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int32_t), pValues);
	}

private:
	uint32_t m_DescriptorSetCount = 0;
	std::vector<DescriptorLayout> m_DescriptorLayout{};
	std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings{};
	VkDescriptorSetLayout m_DescriptorSetLayout{};
	VkPipelineLayout m_PipelineLayout{};
	std::vector<VkDescriptorSet> m_DescriptorSets{};
};