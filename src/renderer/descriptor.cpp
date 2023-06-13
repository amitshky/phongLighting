#include "renderer/descriptor.h"

#include "core/core.h"
#include "renderer/device.h"


DescriptorSet::DescriptorSet(uint32_t descriptorSetCount)
{
	Init(descriptorSetCount);
}

DescriptorSet::~DescriptorSet()
{
	Cleanup();
}

void DescriptorSet::Init(uint32_t descriptorSetCount)
{
	m_DescriptorSetCount = descriptorSetCount;
}

void DescriptorSet::Cleanup()
{
	vkDestroyDescriptorPool(Device::GetDevice(), m_DescriptorPool, nullptr);
	vkDestroyPipelineLayout(Device::GetDevice(), m_PipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(Device::GetDevice(), m_DescriptorSetLayout, nullptr);
}

void DescriptorSet::SetupLayout(std::initializer_list<DescriptorLayout> layout)
{
	m_DescriptorLayout.insert(m_DescriptorLayout.end(), layout);
	m_LayoutBindings.reserve(layout.size());
	for (auto& l : layout)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = l.shaderBinding; // binding in the shader
		layoutBinding.descriptorType = static_cast<VkDescriptorType>(l.descriptorType);
		layoutBinding.descriptorCount = l.bindingDescriptorCount;
		layoutBinding.stageFlags = static_cast<VkShaderStageFlags>(l.shaderStageFlags);
		layoutBinding.pImmutableSamplers = nullptr;

		m_LayoutBindings.push_back(layoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(m_LayoutBindings.size());
	descriptorSetLayoutInfo.pBindings = m_LayoutBindings.data();

	THROW(vkCreateDescriptorSetLayout(Device::GetDevice(), &descriptorSetLayoutInfo, nullptr, &m_DescriptorSetLayout)
			  != VK_SUCCESS,
		"Failed to create descriptor set layout!");

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	THROW(vkCreatePipelineLayout(Device::GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS,
		"Failed to create pipeline layout!");

	CreateDescriptorPool();
}

DescriptorLayout DescriptorSet::CreateLayout(DescriptorType descriptorType,
	ShaderType shaderStage,
	uint32_t shaderBinding,
	uint32_t bindingDescriptorCount,
	UniformBuffer* pUniformBuffer,
	uint32_t uniformBufferCount,
	Texture2D* pTexture,
	uint32_t textureCount)
{
	DescriptorLayout layout{};
	layout.shaderBinding = shaderBinding;
	layout.descriptorType = descriptorType;
	layout.bindingDescriptorCount = bindingDescriptorCount;
	layout.shaderStageFlags = shaderStage;
	layout.pUniformBuffer = pUniformBuffer;
	layout.uniformBufferCount = uniformBufferCount;
	layout.pTexture = pTexture;
	layout.textureCount = textureCount;

	return layout;
}

void DescriptorSet::CreateDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.reserve(m_LayoutBindings.size());

	for (auto& binding : m_LayoutBindings)
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = binding.descriptorType;
		poolSize.descriptorCount = m_DescriptorSetCount;
		poolSizes.push_back(poolSize);
	}

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.maxSets = m_DescriptorSetCount;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();

	THROW(vkCreateDescriptorPool(Device::GetDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS,
		"Failed to create descriptor pool!");
}

void DescriptorSet::Create()
{
	std::vector<VkDescriptorSetLayout> setLayouts{ m_DescriptorSetCount, m_DescriptorSetLayout };

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = m_DescriptorSetCount;
	descriptorSetAllocateInfo.pSetLayouts = setLayouts.data();

	// we create one descriptor set for each frame with the same layout
	m_DescriptorSets.resize(m_DescriptorSetCount);
	THROW(vkAllocateDescriptorSets(Device::GetDevice(), &descriptorSetAllocateInfo, m_DescriptorSets.data())
			  != VK_SUCCESS,
		"Failed to allocate descriptor sets!")

	for (uint64_t i = 0; i < m_DescriptorSetCount; ++i)
	{
		std::vector<VkWriteDescriptorSet> descriptorWrites{};
		descriptorWrites.reserve(m_DescriptorSetCount);

		for (auto& layout : m_DescriptorLayout)
		{
			VkWriteDescriptorSet descWrite{};
			descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descWrite.dstSet = m_DescriptorSets[i];
			descWrite.dstBinding = layout.shaderBinding;
			descWrite.dstArrayElement = 0;
			descWrite.descriptorType = static_cast<VkDescriptorType>(layout.descriptorType);
			descWrite.descriptorCount = layout.bindingDescriptorCount;
			descWrite.pBufferInfo = layout.pUniformBuffer == nullptr
										? nullptr
										: &(layout.pUniformBuffer[i % layout.uniformBufferCount].GetBufferInfo());
			descWrite.pImageInfo =
				layout.pTexture == nullptr ? nullptr : &(layout.pTexture[i % layout.textureCount].GetImageInfo());

			descriptorWrites.push_back(descWrite);
		}

		vkUpdateDescriptorSets(
			Device::GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void DescriptorSet::Bind(VkCommandBuffer commandBuffer,
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