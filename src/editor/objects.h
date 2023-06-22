#pragma once

#include <vulkan/vulkan.h>
#include "renderer/vertexBuffer.h"
#include "renderer/indexBuffer.h"
#include "renderer/texture.h"
#include "renderer/pipeline.h"
#include "renderer/descriptor.h"
#include "editor/ubo.h"


class Cube
{
public:
	Cube(VkRenderPass renderPass, const uint32_t maxFramesInFlight, const uint64_t numInstances);

	void Draw(VkCommandBuffer commandBuffer,
		const uint64_t currentFrameIndex,
		const uint32_t dynamicOffsetCount,
		const uint32_t* dynamicOffset);

	void UpdateUniformBuffers(const UniformBufferObject& ubo,
		const DynamicUniformBufferObject& dUbo,
		const uint32_t currentFrameIndex);

private:
	uint64_t m_DUboAlignmentSize = 0;

	std::unique_ptr<VertexBuffer> m_VertexBuffer;
	std::unique_ptr<IndexBuffer> m_IndexBuffer;
	std::vector<Texture2D> m_Textures;

	std::vector<UniformBuffer> m_UniformBuffers{};
	std::vector<UniformBuffer> m_DynamicUniformBuffers{};
	std::unique_ptr<DescriptorSet> m_DescriptorSet{};
	std::unique_ptr<Pipeline> m_Pipeline{};
};


class LightCube
{
public:
	LightCube(VkRenderPass renderPass, const uint32_t maxFramesInFlight, const uint64_t numInstances);

	void Draw(VkCommandBuffer commandBuffer, const uint64_t currentFrameIndex);

	void UpdateUniformBuffers(const LightCubeUBO& ubo, const uint32_t currentFrameIndex);

private:
	std::unique_ptr<VertexBuffer> m_VertexBuffer;
	std::unique_ptr<IndexBuffer> m_IndexBuffer;

	std::vector<UniformBuffer> m_UniformBuffers{};
	std::unique_ptr<DescriptorSet> m_DescriptorSet{};
	std::unique_ptr<Pipeline> m_Pipeline{};
};
