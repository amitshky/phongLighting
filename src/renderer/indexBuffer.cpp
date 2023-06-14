#include "renderer/indexBuffer.h"

#include "utils/utils.h"
#include "renderer/device.h"


IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices)
	: m_IndexSize{ static_cast<uint32_t>(indices.size()) }
{
	Init(indices);
}

IndexBuffer::~IndexBuffer()
{
	Cleanup();
}

void IndexBuffer::Init(const std::vector<uint32_t>& indices)
{
	VkDeviceSize size = sizeof(indices[0]) * static_cast<uint64_t>(indices.size());

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	utils::CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMem);

	void* data;
	vkMapMemory(Device::GetDevice(), stagingBufferMem, 0, size, 0, &data);
	memcpy(data, indices.data(), static_cast<size_t>(size));
	vkUnmapMemory(Device::GetDevice(), stagingBufferMem);

	// create the actual index buffer
	utils::CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_IndexBuffer,
		m_IndexBufferMemory);

	utils::CopyBuffer(stagingBuffer, m_IndexBuffer, size);

	vkFreeMemory(Device::GetDevice(), stagingBufferMem, nullptr);
	vkDestroyBuffer(Device::GetDevice(), stagingBuffer, nullptr);
}

void IndexBuffer::Cleanup()
{
	vkFreeMemory(Device::GetDevice(), m_IndexBufferMemory, nullptr);
	vkDestroyBuffer(Device::GetDevice(), m_IndexBuffer, nullptr);
}