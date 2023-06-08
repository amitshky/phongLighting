#version 450

layout(binding = 0) uniform UniformBufferObject
{
	vec3 lightPos;
	vec3 viewPos;
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 normMat;
}
ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outFragPos;
layout(location = 3) out vec3 outViewPos;
layout(location = 4) out vec3 outLightPos;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	// we cannot simply multiply the normal vector by the model matrix,
	// because we shouldnt translate the normal vector
	// we use a normal matrix
	outNormal = mat3(ubo.normMat) * inNormal;

	outTexCoord = inTexCoord;
	outFragPos = vec3(ubo.model * vec4(inPosition, 1.0));
	outViewPos = ubo.viewPos;
	outLightPos = ubo.lightPos;
}