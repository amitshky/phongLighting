#pragma once

#include <vulkan/vulkan.h>
#include <cstring>


class UniformBuffer
{
public:
	UniformBuffer(VkDeviceSize size, VkMemoryPropertyFlags properties, VkDeviceSize descriptorUpdateRange);
	~UniformBuffer();

	inline VkDescriptorBufferInfo& GetBufferInfo() { return m_BufferInfo; }
	inline void Map(const void* src) { memcpy(m_UniformBufferMapped, src, m_BufferSize); }

private:
	VkDeviceSize m_BufferSize;
	VkDescriptorBufferInfo m_BufferInfo{};
	VkBuffer m_UniformBuffer{};
	VkDeviceMemory m_UniformBufferMemory{};
	void* m_UniformBufferMapped{};
};
