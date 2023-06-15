#include "renderer/commandBuffer.h"

#include "core/core.h"
#include "renderer/device.h"
#include "renderer/commandPool.h"


CommandBuffer::CommandBuffer(uint64_t cmdBuffCount)
{
	Init(cmdBuffCount);
}

void CommandBuffer::Init(uint64_t cmdBuffCount)
{
	m_CommandBuffers.resize(cmdBuffCount);

	VkCommandBufferAllocateInfo cmdBuffAllocInfo{};
	cmdBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBuffAllocInfo.commandPool = CommandPool::Get();
	cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBuffAllocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	THROW(vkAllocateCommandBuffers(Device::GetDevice(), &cmdBuffAllocInfo, m_CommandBuffers.data()) != VK_SUCCESS,
		"Failed to allocate command buffers!")
}

void CommandBuffer::Begin(uint32_t currentFrameIdx)
{
	Reset(currentFrameIdx);
	VkCommandBufferBeginInfo cmdBuffBeginInfo{};
	cmdBuffBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	THROW(vkBeginCommandBuffer(m_CommandBuffers[currentFrameIdx], &cmdBuffBeginInfo) != VK_SUCCESS,
		"Failed to begin recording command buffer!")
}

void CommandBuffer::End(uint32_t currentFrameIdx)
{
	THROW(vkEndCommandBuffer(m_CommandBuffers[currentFrameIdx]) != VK_SUCCESS, "Failed to record command buffer!");
}

void CommandBuffer::Submit(VkFence inFlightFence,
	const VkSemaphore* pWaitSemaphores,
	uint32_t waitSemaphoreCount,
	const VkSemaphore* pSignalSemaphores,
	uint32_t signalSemaphoreCount,
	const VkPipelineStageFlags* pWaitStages,
	uint32_t currentFrameIdx)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = waitSemaphoreCount;
	submitInfo.pWaitSemaphores = pWaitSemaphores;
	submitInfo.pWaitDstStageMask = pWaitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[currentFrameIdx];
	submitInfo.signalSemaphoreCount = signalSemaphoreCount;
	submitInfo.pSignalSemaphores = pSignalSemaphores;

	// signals the fence after executing the command buffer
	THROW(vkQueueSubmit(Device::GetGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS,
		"Failed to submit draw command buffer!")
}

void CommandBuffer::Reset(uint32_t currentFrameIdx)
{
	vkResetCommandBuffer(m_CommandBuffers[currentFrameIdx], 0);
}

VkCommandBuffer CommandBuffer::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo cmdBuffAllocInfo{};
	cmdBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBuffAllocInfo.commandPool = CommandPool::Get();
	cmdBuffAllocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuff;
	vkAllocateCommandBuffers(Device::GetDevice(), &cmdBuffAllocInfo, &cmdBuff);

	// immediately start recording the command buffer
	VkCommandBufferBeginInfo cmdBuffBegin{};
	cmdBuffBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBuffBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuff, &cmdBuffBegin);

	return cmdBuff;
}

void CommandBuffer::EndSingleTimeCommands(VkCommandBuffer cmdBuff)
{
	vkEndCommandBuffer(cmdBuff);

	// submit the queue
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuff;

	vkQueueSubmit(Device::GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(Device::GetGraphicsQueue());

	vkFreeCommandBuffers(Device::GetDevice(), CommandPool::Get(), 1, &cmdBuff);
}