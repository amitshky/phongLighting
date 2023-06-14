#pragma once

#include <vector>
#include <vulkan/vulkan.h>


class IndexBuffer
{
public:
	IndexBuffer(const std::vector<uint32_t>& indices);
	~IndexBuffer();

	inline VkBuffer GetBuffer() const { return m_IndexBuffer; }
	inline void Draw(VkCommandBuffer commandBuffer) { vkCmdDrawIndexed(commandBuffer, m_IndexSize, 1, 0, 0, 0); }
	inline void Bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

private:
	void Init(const std::vector<uint32_t>& indices);
	void Cleanup();

private:
	uint32_t m_IndexSize;
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;
};