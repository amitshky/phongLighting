#pragma once

#include <vulkan/vulkan.h>

class Pipeline
{
public:
	Pipeline(const char* vertShaderPath,
		const char* fragShaderPath,
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass);
	~Pipeline();

	inline void Bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	}

private:
	void Init(const char* vertShaderPath,
		const char* fragShaderPath,
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass);
	void Cleanup();

private:
	VkPipeline m_Pipeline{};
};