#version 450

layout(binding = 1) uniform sampler2D uTexSampler;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inFragPos;
layout(location = 3) in vec3 inViewPos;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 lightPos = vec3(0.0, 0.0, 0.8);
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	// ambient light
	float ambientStrength = 0.01;
	vec4 ambientColor = vec4(lightColor * ambientStrength, 1.0);

	// diffuse light
	float diffuseStrength = 0.5;
	vec3 norm = normalize(inNormal);
	vec3 lightDir = normalize(lightPos - inFragPos);
	vec4 diffuseColor = vec4(diffuseStrength * max(dot(lightDir, norm), 0.0) * lightColor, 1.0);

	// spcular light
	float specularStrength = 1.0;
	int shininess = 64;
	vec3 viewDir = normalize(inViewPos - inFragPos);
	vec3 reflectDir = reflect(-lightDir, norm); // light direction should be towards the point
	vec4 specularColor = vec4(specularStrength * pow(max(dot(reflectDir, viewDir), 0.0), shininess) * lightColor, 1.0);

	vec4 cubeColor = texture(uTexSampler, inTexCoord);
	outColor = (ambientColor + diffuseColor + specularColor) * cubeColor;
	// outColor = vec4(inTexCoord, 0.0, 1.0);
}
