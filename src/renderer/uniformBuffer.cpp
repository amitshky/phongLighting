#include "renderer/uniformBuffer.h"

#include "renderer/device.h"
#include "utils/utils.h"


UniformBuffer::UniformBuffer(VkDeviceSize size, VkMemoryPropertyFlags properties, VkDeviceSize descriptorUpdateRange)
	: m_BufferSize{ size }
{
	utils::CreateBuffer(
		m_BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, properties, m_UniformBuffer, m_UniformBufferMemory);
	vkMapMemory(Device::GetDevice(), m_UniformBufferMemory, 0, m_BufferSize, 0, &m_UniformBufferMapped);

	m_BufferInfo.buffer = m_UniformBuffer;
	m_BufferInfo.offset = 0;
	m_BufferInfo.range = descriptorUpdateRange;
}

UniformBuffer::~UniformBuffer()
{
	vkUnmapMemory(Device::GetDevice(), m_UniformBufferMemory);
	vkFreeMemory(Device::GetDevice(), m_UniformBufferMemory, nullptr);
	vkDestroyBuffer(Device::GetDevice(), m_UniformBuffer, nullptr);
}
