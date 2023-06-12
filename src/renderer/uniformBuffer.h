#pragma once

#include <glm/glm.hpp>

// TODO: move these to renderer.cpp or somewhere else cuz
// it is not so general

struct UniformBufferObject
{
	// explicitly speicify alignments
	// because vulkan expects structs to be in a specific alignment with the
	// structs in the shaders
	// scalars = 4bytes
	// vec2 = 8bytes
	// vec3 / vec4 = 16bytes
	// mat4 = 16bytes

	// NOTE: alignas() can be removed in this case because they are all 16bytes
	alignas(16) glm::vec3 lightPos;
	alignas(16) glm::vec3 viewPos;
	alignas(16) glm::mat4 viewMat;
	alignas(16) glm::mat4 projMat;
	// alignas(16) glm::mat4 normMat; // for normal vector
};

struct DynamicUniformBufferObject
{
public:
	glm::mat4* modelMat = nullptr;
	// glm::mat4 normMat; // for normal vector

public:
	void Init(uint64_t minAlignmentSize, const uint64_t numCubes)
	{
		m_AlignmentSize = sizeof(glm::mat4);
		if (minAlignmentSize > 0)
		{
			// return value greater than `minAlignmentSize - 1`
			m_AlignmentSize = (m_AlignmentSize + minAlignmentSize - 1) & ~(minAlignmentSize - 1);
		}

		m_Size = numCubes * m_AlignmentSize;
		modelMat = static_cast<glm::mat4*>(_aligned_malloc(m_Size, m_AlignmentSize));
	}

	void Cleanup() { _aligned_free(modelMat); }

	inline const uint64_t GetBufferSize() const { return m_Size; }
	inline const uint64_t GetAlignment() const { return m_AlignmentSize; }

	glm::mat4* operator[](uint64_t index) const
	{
		return reinterpret_cast<glm::mat4*>(reinterpret_cast<uint64_t>(modelMat) + (index * m_AlignmentSize));
	}

private:
	uint64_t m_AlignmentSize = 0;
	uint64_t m_Size = 0;
};
