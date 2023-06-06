#pragma once

#include <glm/glm.hpp>

struct UniformBufferObject
{
	// explicitly speicify alignments
	// because vulkan expects structs to be in a specific alignment with the
	// structs in the shaders
	alignas(16) glm::vec3 viewPos;
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};
