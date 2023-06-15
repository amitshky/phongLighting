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
		const VkSemaphore* pWaitSemaphores,
		uint32_t waitSemaphoreCount,
		const VkSemaphore* pSignalSemaphores,
		uint32_t signalSemaphoreCount,
		const VkPipelineStageFlags* pWaitStages,
		uint32_t currentFrameIdx);
	void Reset(uint32_t currentFrameIdx);

	static VkCommandBuffer BeginSingleTimeCommands();
	static void EndSingleTimeCommands(VkCommandBuffer cmdBuff);

	inline VkCommandBuffer GetBufferAt(uint32_t currentFrameIndex) const { return m_CommandBuffers[currentFrameIndex]; }

private:
	void Init(uint64_t cmdBuffCount);

private:
	std::vector<VkCommandBuffer> m_CommandBuffers;
};