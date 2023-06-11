#pragma once

#include <vector>
#include <vulkan/vulkan.h>


enum class ShaderType {
	VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
	FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
	COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT
};

class Shader
{
public:
	Shader(const char* path, ShaderType type);
	~Shader();

	inline VkPipelineShaderStageCreateInfo GetShaderStage() const { return m_ShaderStage; }

private:
	void LoadShader();
	void CreateShaderModule();
	void CreateShaderStage();


private:
	const char* m_Path;
	ShaderType m_Type;

	std::vector<char> m_ShaderCode;

	VkShaderModule m_ShaderModule;
	VkPipelineShaderStageCreateInfo m_ShaderStage;
};
