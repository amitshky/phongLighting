#version 450

layout(binding = 0) uniform UniformBufferObject
{
	vec3 viewPos;
	mat4 model;
	mat4 view;
	mat4 proj;
}
ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outFragPos;
layout(location = 3) out vec3 outViewPos;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	// we cannot simply multiply the normal vector by the model matrix,
	// because we shouldnt translate the normal vector
	// we use a normal matrix
	// TODO: calc the normal matrix in the cpu side because
	// inverse operation is costly
	outNormal = mat3(transpose(inverse(ubo.model))) * inNormal;

	outTexCoord = inTexCoord;
	outFragPos = vec3(ubo.model * vec4(inPosition, 1.0));
	outViewPos = ubo.viewPos;
}