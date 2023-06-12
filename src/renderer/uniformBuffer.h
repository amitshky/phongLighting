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

	alignas(16) glm::vec3 lightPos;
	alignas(16) glm::vec3 viewPos;
	alignas(16) glm::mat4 viewProjMat;
};

class DynamicUniformBufferObject
{
public:
	// the buffer contains model and normal matrices
	// they are aligned one after another in the memory
	// the buffer is allocated using `_aligned_malloc`
	// each of the alignments are bound one at a time
	// so only one model and normal matrix is sent to the shader
	// eg if alignment is 256bytes (and sizeof(glm::mat4) = 64bytes):
	// |---model-1---|---normal-1--|-------------|-------------|
	// |---model-2---|---normal-2--|-------------|-------------|
	// |---model-3---|---normal-3--|-------------|-------------|
	glm::mat4* buffer = nullptr;

public:
	~DynamicUniformBufferObject() { Cleanup(); }

	void Init(uint64_t minAlignmentSize, const uint64_t numCubes)
	{
		m_AlignmentSize = sizeof(glm::mat4) * 2; // size of model and normal matrices
		if (minAlignmentSize > 0)
		{
			// returns value greater than `minAlignmentSize - 1` and greater or equal to `m_AlignmentSize`, and is a
			// power of 2
			m_AlignmentSize = (m_AlignmentSize + minAlignmentSize - 1) & ~(minAlignmentSize - 1);
		}

		m_Size = numCubes * m_AlignmentSize;
		buffer = static_cast<glm::mat4*>(_aligned_malloc(m_Size, m_AlignmentSize));
	}

	void Cleanup()
	{
		if (buffer == nullptr)
			return;

		_aligned_free(buffer);
		buffer = nullptr;
	}

	inline const uint64_t GetBufferSize() const { return m_Size; }
	inline const uint64_t GetAlignment() const { return m_AlignmentSize; }

	inline glm::mat4* GetModelMatPtr(uint64_t index) const
	{
		return reinterpret_cast<glm::mat4*>(reinterpret_cast<uint64_t>(buffer) + (index * m_AlignmentSize));
	}
	inline glm::mat4* GetNormalMatPtr(uint64_t index) const
	{
		return reinterpret_cast<glm::mat4*>(
			reinterpret_cast<uint64_t>(buffer) + sizeof(glm::mat4) + (index * m_AlignmentSize));
	}

private:
	uint64_t m_AlignmentSize = 0;
	uint64_t m_Size = 0;
};
