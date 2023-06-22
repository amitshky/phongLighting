#version 450

layout(binding = 2) uniform texture2D uTextures[2];
layout(binding = 3) uniform sampler uSampler;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inFragPos;
layout(location = 3) in vec3 inViewPos;
layout(location = 4) in vec3 inLightPos;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 diffuseTex = texture(sampler2D(uTextures[0], uSampler), inTexCoord);
	// vec4 specularTex = texture(sampler2D(uTextures[1], uSampler), inTexCoord);

	outColor = vec4(pow(diffuseTex.rgb, vec3(1.0 / 2.2)), 1.0);
}
