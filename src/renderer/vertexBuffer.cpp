#include "renderer/vertexBuffer.h"

#include "utils/utils.h"
#include "renderer/device.h"


VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices, const std::vector<VkDeviceSize>& offsets)
	: m_VertexSize{ static_cast<uint32_t>(vertices.size()) },
	  m_VertexOffsets{ offsets }
{
	Init(vertices);
}

VertexBuffer::~VertexBuffer()
{
	Cleanup();
}

void VertexBuffer::Init(const std::vector<Vertex>& vertices)
{
	VkDeviceSize size = sizeof(vertices[0]) * static_cast<uint64_t>(vertices.size());

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	utils::CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMem);

	void* data;
	vkMapMemory(Device::GetDevice(), stagingBufferMem, 0, size, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(size));
	vkUnmapMemory(Device::GetDevice(), stagingBufferMem);

	// create the actual vertex buffer
	utils::CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_VertexBuffer,
		m_VertexBufferMemory);

	utils::CopyBuffer(stagingBuffer, m_VertexBuffer, size);

	vkFreeMemory(Device::GetDevice(), stagingBufferMem, nullptr);
	vkDestroyBuffer(Device::GetDevice(), stagingBuffer, nullptr);
}

void VertexBuffer::Cleanup()
{
	vkFreeMemory(Device::GetDevice(), m_VertexBufferMemory, nullptr);
	vkDestroyBuffer(Device::GetDevice(), m_VertexBuffer, nullptr);
}