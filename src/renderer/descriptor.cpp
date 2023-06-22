#include "renderer/descriptor.h"

#include <array>
#include "core/core.h"
#include "renderer/device.h"


VkDescriptorPool DescriptorPool::s_DescriptorPool{};

void DescriptorPool::Init()
{
	VkDescriptorPoolSize poolSizes[] = {
		{VK_DESCRIPTOR_TYPE_SAMPLER,                 1000},
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolInfo.maxSets = 1000;
	descriptorPoolInfo.poolSizeCount = std::size(poolSizes);
	descriptorPoolInfo.pPoolSizes = poolSizes;

	THROW(vkCreateDescriptorPool(Device::GetDevice(), &descriptorPoolInfo, nullptr, &s_DescriptorPool) != VK_SUCCESS,
		"Failed to create descriptor pool!");
}

void DescriptorPool::Cleanup()
{
	vkDestroyDescriptorPool(Device::GetDevice(), s_DescriptorPool, nullptr);
}


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
		layoutBinding.descriptorCount = l.descriptorCount;
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

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(int32_t);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	THROW(vkCreatePipelineLayout(Device::GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS,
		"Failed to create pipeline layout!");
}

void DescriptorSet::Create()
{
	std::vector<VkDescriptorSetLayout> setLayouts{ m_DescriptorSetCount, m_DescriptorSetLayout };

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = DescriptorPool::Get();
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

		descriptorWrites.reserve(m_DescriptorLayout.size());
		for (auto& layout : m_DescriptorLayout)
		{
			VkWriteDescriptorSet descWrite{};
			descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descWrite.dstSet = m_DescriptorSets[i];
			descWrite.dstBinding = layout.shaderBinding;
			descWrite.dstArrayElement = 0;
			descWrite.descriptorType = static_cast<VkDescriptorType>(layout.descriptorType);
			descWrite.descriptorCount = layout.descriptorCount;
			descWrite.pBufferInfo = layout.pBufferInfos == nullptr ? nullptr : &layout.pBufferInfos[i];
			descWrite.pImageInfo = layout.pImageInfos == nullptr ? nullptr : layout.pImageInfos;

			descriptorWrites.push_back(descWrite);
		}

		vkUpdateDescriptorSets(
			Device::GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

DescriptorLayout DescriptorSet::CreateLayout(DescriptorType descriptorType,
	ShaderType shaderStage,
	uint32_t shaderBinding,
	uint32_t descriptorCount,
	VkDescriptorBufferInfo* pBufferInfos,
	VkDescriptorImageInfo* pImageInfos)
{
	DescriptorLayout layout{};
	layout.shaderBinding = shaderBinding;
	layout.descriptorType = descriptorType;
	layout.descriptorCount = descriptorCount;
	layout.shaderStageFlags = shaderStage;
	layout.pBufferInfos = pBufferInfos;
	layout.pImageInfos = pImageInfos;

	return layout;
}
