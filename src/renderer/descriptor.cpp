#include "renderer/descriptor.h"

#include <array>
#include "core/core.h"
#include "renderer/device.h"


VkDescriptorPool DescriptorSet::s_DescriptorPool{};

DescriptorSet::DescriptorSet(uint32_t descriptorSetCount)
{
	Init(descriptorSetCount);
}

DescriptorSet::~DescriptorSet()
{
	Cleanup();
}

void DescriptorSet::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSizes[] = {
		{VK_DESCRIPTOR_TYPE_SAMPLER,                 100},
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          100},
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          100},
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   100},
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   100},
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         100},
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         100},
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       100}
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolInfo.maxSets = 100;
	descriptorPoolInfo.poolSizeCount = std::size(poolSizes);
	descriptorPoolInfo.pPoolSizes = poolSizes;

	THROW(vkCreateDescriptorPool(Device::GetDevice(), &descriptorPoolInfo, nullptr, &s_DescriptorPool) != VK_SUCCESS,
		"Failed to create descriptor pool!");
}

void DescriptorSet::Init(uint32_t descriptorSetCount)
{
	m_DescriptorSetCount = descriptorSetCount;
}

void DescriptorSet::Cleanup()
{
	vkDestroyDescriptorPool(Device::GetDevice(), s_DescriptorPool, nullptr);
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

	// // uniform buffer
	// VkDescriptorSetLayoutBinding uboLayout{};
	// uboLayout.binding = 0; // binding in the shader
	// uboLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	// uboLayout.descriptorCount = 1;
	// uboLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	// uboLayout.pImmutableSamplers = nullptr;
	// // dynamic ubo for per-object data
	// VkDescriptorSetLayoutBinding dUboLayout{};
	// dUboLayout.binding = 1; // binding in the shader
	// dUboLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	// dUboLayout.descriptorCount = 1;
	// dUboLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	// dUboLayout.pImmutableSamplers = nullptr;

	// // combined image sampler // for textures
	// VkDescriptorSetLayoutBinding imgSamplerLayout{};
	// imgSamplerLayout.binding = 2; // binding in the shader
	// imgSamplerLayout.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	// imgSamplerLayout.descriptorCount = 1;
	// imgSamplerLayout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	// imgSamplerLayout.pImmutableSamplers = nullptr;

	// std::array<VkDescriptorSetLayoutBinding, 3> bindings{ uboLayout, dUboLayout, imgSamplerLayout };

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

void DescriptorSet::Create()
{
	std::vector<VkDescriptorSetLayout> setLayouts{ m_DescriptorSetCount, m_DescriptorSetLayout };

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = s_DescriptorPool;
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
			descWrite.descriptorCount = layout.bindingDescriptorCount;
			descWrite.pBufferInfo = layout.pUniformBuffer == nullptr
										? nullptr
										: &(layout.pUniformBuffer[i % layout.uniformBufferCount].GetBufferInfo());
			descWrite.pImageInfo =
				layout.pTexture == nullptr ? nullptr : &(layout.pTexture[i % layout.textureCount].GetImageInfo());

			descriptorWrites.push_back(descWrite);
		}


		// descriptorWrites.resize(m_DescriptorLayout.size());
		// descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// descriptorWrites[0].dstSet = m_DescriptorSets[i];
		// descriptorWrites[0].dstBinding = 0;
		// descriptorWrites[0].dstArrayElement = 0;
		// descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		// descriptorWrites[0].descriptorCount = 1;
		// descriptorWrites[0].pBufferInfo =
		// 	&(m_DescriptorLayout[0].pUniformBuffer[i % m_DescriptorLayout[0].uniformBufferCount].GetBufferInfo());
		// // for dynamic ubo
		// descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// descriptorWrites[1].dstSet = m_DescriptorSets[i];
		// descriptorWrites[1].dstBinding = 1;
		// descriptorWrites[1].dstArrayElement = 0;
		// descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		// descriptorWrites[1].descriptorCount = 1;
		// descriptorWrites[1].pBufferInfo =
		// 	&(m_DescriptorLayout[1].pUniformBuffer[i % m_DescriptorLayout[1].uniformBufferCount].GetBufferInfo());
		// // combined image sampler // for textures
		// descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// descriptorWrites[2].dstSet = m_DescriptorSets[i];
		// descriptorWrites[2].dstBinding = 2;
		// descriptorWrites[2].dstArrayElement = 0;
		// descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		// descriptorWrites[2].descriptorCount = 1;
		// descriptorWrites[2].pImageInfo =
		// 	&(m_DescriptorLayout[2].pTexture[i % m_DescriptorLayout[2].textureCount].GetImageInfo());

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