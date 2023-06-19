#version 450

layout(binding = 0) uniform LightCubeUniformBuffer
{
	mat4 transformationMat;
}
ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal; // unused
layout(location = 2) in vec2 inTexCoord; // unused

void main()
{
	gl_Position = ubo.transformationMat * vec4(inPosition, 1.0);
}