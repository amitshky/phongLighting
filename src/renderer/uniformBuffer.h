#pragma once

#include <glm/glm.hpp>

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
	alignas(16) glm::mat4 modelMat;
	alignas(16) glm::mat4 viewMat;
	alignas(16) glm::mat4 projMat;
	alignas(16) glm::mat4 normMat; // for normal vector
};
