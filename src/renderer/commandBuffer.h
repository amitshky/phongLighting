#pragma once

#include <vector>
#include <vulkan/vulkan.h>


class CommandBuffer
{
public:
	CommandBuffer(uint64_t cmdBuffCount);

	void Begin(uint32_t currentFrameIdx);
	void End(uint32_t currentFrameIdx);
	void Submit(VkFence inFlightFence,
		const VkSemaphore* waitSemaphores,
		uint32_t waitSemaphoreCount,
		const VkSemaphore* signalSemaphores,
		uint32_t signalSemaphoreCount,
		const VkPipelineStageFlags* waitStages,
		uint32_t currentFrameIdx);
	void Reset(uint32_t currentFrameIdx);

	static VkCommandBuffer BeginSingleTimeCommands();
	static void EndSingleTimeCommands(VkCommandBuffer cmdBuff);

	inline VkCommandBuffer* GetBuffers() { return m_CommandBuffers.data(); }
	// inline VkCommandBuffer& GetBufferAt(uint32_t currentFrameIndex) { return m_CommandBuffers[currentFrameIndex]; }

private:
	void Init(uint64_t cmdBuffCount);

private:
	std::vector<VkCommandBuffer> m_CommandBuffers;
};