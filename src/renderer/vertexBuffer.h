#pragma once

#include <array>
#include <vector>
#include <functional>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>


struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(Vertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDesc;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 3> attrDesc{};
		// for position
		attrDesc[0].location = 0;
		attrDesc[0].binding = 0;
		attrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc[0].offset = offsetof(Vertex, pos);
		// for color
		attrDesc[1].location = 1;
		attrDesc[1].binding = 0;
		attrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc[1].offset = offsetof(Vertex, normal);
		// for texture coordinates
		attrDesc[2].location = 2;
		attrDesc[2].binding = 0;
		attrDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
		attrDesc[2].offset = offsetof(Vertex, texCoord);

		return attrDesc;
	}

	// for hash function
	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
	}
};

// hash function
namespace std {
template<>
struct hash<Vertex>
{
	size_t operator()(Vertex const& vertex) const
	{
		return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1)
			   ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};
} // namespace std


class VertexBuffer
{
public:
	VertexBuffer(const std::vector<Vertex>& vertices, const std::vector<VkDeviceSize>& offsets);
	~VertexBuffer();

	inline VkBuffer GetBuffer() const { return m_VertexBuffer; }
	inline void Draw(VkCommandBuffer commandBuffer) { vkCmdDraw(commandBuffer, m_VertexSize, 1, 0, 0); }
	inline void Bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindVertexBuffers(
			commandBuffer, 0, static_cast<uint32_t>(m_VertexOffsets.size()), &m_VertexBuffer, m_VertexOffsets.data());
	}

private:
	void Init(const std::vector<Vertex>& vertices);
	void Cleanup();

private:
	uint32_t m_VertexSize;
	std::vector<VkDeviceSize> m_VertexOffsets;
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
};