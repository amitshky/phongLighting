#version 450

// layout(binding = 2) uniform sampler2D uTexSampler;
layout(binding = 2) uniform texture2D uTextures[3];
layout(binding = 3) uniform sampler uSampler;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inFragPos;
layout(location = 3) in vec3 inViewPos;
layout(location = 4) in vec3 inLightPos;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PerObject
{
	int imgIndex;
}
pc;

void main()
{
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	// ambient light
	float ambientStrength = 0.01;
	vec3 ambientLight = lightColor * ambientStrength;

	// diffuse light
	float diffuseStrength = 0.4;
	vec3 norm = normalize(inNormal);
	vec3 lightDir = normalize(inLightPos - inFragPos);
	vec3 diffuseLight = diffuseStrength * max(dot(lightDir, norm), 0.0) * lightColor;

	// spcular light
	float specularStrength = 1.0;
	int shininess = 64;
	vec3 viewDir = normalize(inViewPos - inFragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	vec3 specularLight = specularStrength * pow(max(dot(norm, halfwayDir), 0.0), shininess) * lightColor;

	vec3 cubeColor = texture(sampler2D(uTextures[pc.imgIndex], uSampler), inTexCoord).rgb;
	vec3 result = (ambientLight + diffuseLight + specularLight) * cubeColor;
	result = pow(result.rgb, vec3(1.0 / 2.2)); // because window surface format = VK_FORMAT_B8G8R8A8_UNORM
	outColor = vec4(result, 1.0);
	// outColor = vec4(inTexCoord, 0.0, 1.0);
}
