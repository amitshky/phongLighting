#version 450

layout(binding = 0) uniform UniformBufferObject
{
	vec3 lightPos;
	vec3 viewPos;
	mat4 viewProjMat;
}
ubo;
layout(binding = 1) uniform DynamicUniformBufferObject
{
	mat4 modelMat;
	mat4 normMat;
}
dUbo;

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
	outFragPos = vec3(dUbo.modelMat * vec4(inPosition, 1.0));
	gl_Position = ubo.viewProjMat * vec4(outFragPos, 1.0);

	// we cannot simply multiply the normal vector by the model matrix,
	// because we shouldnt translate the normal vector
	// we use a normal matrix
	outNormal = mat3(dUbo.normMat) * inNormal;

	outTexCoord = inTexCoord;
	outViewPos = ubo.viewPos;
	outLightPos = ubo.lightPos;
}