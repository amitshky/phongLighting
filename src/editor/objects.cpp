#include "editor/objects.h"

#include "renderer/device.h"
#include "utils/utils.h"


const std::vector<Vertex> vertexData{
  // front
	{{ -0.5f, 0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f }}, // 3
	{ { -0.5f, -0.5f, 0.5f },  { 0.0f, 0.0f, 1.0f },  { 0.0f, 1.0f }}, // 0
	{ { 0.5f, -0.5f, 0.5f },   { 0.0f, 0.0f, 1.0f },  { 1.0f, 1.0f }}, // 1
	{ { 0.5f, -0.5f, 0.5f },   { 0.0f, 0.0f, 1.0f },  { 1.0f, 1.0f }}, // 1
	{ { 0.5f, 0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f },  { 1.0f, 0.0f }}, // 2
	{ { -0.5f, 0.5f, 0.5f },   { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f }}, // 3

  // right
	{ { 0.5f, 0.5f, 0.5f },    { 1.0f, 0.0f, 0.0f },  { 0.0f, 0.0f }}, // 2
	{ { 0.5f, -0.5f, 0.5f },   { 1.0f, 0.0f, 0.0f },  { 0.0f, 1.0f }}, // 1
	{ { 0.5f, -0.5f, -0.5f },  { 1.0f, 0.0f, 0.0f },  { 1.0f, 1.0f }}, // 5
	{ { 0.5f, -0.5f, -0.5f },  { 1.0f, 0.0f, 0.0f },  { 1.0f, 1.0f }}, // 5
	{ { 0.5f, 0.5f, -0.5f },   { 1.0f, 0.0f, 0.0f },  { 1.0f, 0.0f }}, // 6
	{ { 0.5f, 0.5f, 0.5f },    { 1.0f, 0.0f, 0.0f },  { 0.0f, 0.0f }}, // 2

  // back
	{ { 0.5f, 0.5f, -0.5f },   { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }}, // 6
	{ { 0.5f, -0.5f, -0.5f },  { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }}, // 5
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }}, // 4
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }}, // 4
	{ { -0.5f, 0.5f, -0.5f },  { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }}, // 7
	{ { 0.5f, 0.5f, -0.5f },   { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }}, // 6

  // left
	{ { -0.5f, 0.5f, -0.5f },  { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}, // 7
	{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }}, // 4
	{ { -0.5f, -0.5f, 0.5f },  { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }}, // 0
	{ { -0.5f, -0.5f, 0.5f },  { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }}, // 0
	{ { -0.5f, 0.5f, 0.5f },   { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }}, // 3
	{ { -0.5f, 0.5f, -0.5f },  { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}, // 7

  // top
	{ { -0.5f, 0.5f, -0.5f },  { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f }}, // 7
	{ { -0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f },  { 0.0f, 1.0f }}, // 3
	{ { 0.5f, 0.5f, 0.5f },    { 0.0f, 1.0f, 0.0f },  { 1.0f, 1.0f }}, // 2
	{ { 0.5f, 0.5f, 0.5f },    { 0.0f, 1.0f, 0.0f },  { 1.0f, 1.0f }}, // 2
	{ { 0.5f, 0.5f, -0.5f },   { 0.0f, 1.0f, 0.0f },  { 1.0f, 0.0f }}, // 6
	{ { -0.5f, 0.5f, -0.5f },  { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f }}, // 7

  // bottom
	{ { -0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }}, // 0
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }}, // 4
	{ { 0.5f, -0.5f, -0.5f },  { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}, // 5
	{ { 0.5f, -0.5f, -0.5f },  { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}, // 5
	{ { 0.5f, -0.5f, 0.5f },   { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }}, // 1
	{ { -0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }}, // 0
};

const auto [indices, vertices] = utils::GetModelData(vertexData);


Cube::Cube(VkRenderPass renderPass, const uint32_t maxFramesInFlight, const uint64_t numInstances)
{
	m_VertexBuffer = std::make_unique<VertexBuffer>(vertices);
	m_IndexBuffer = std::make_unique<IndexBuffer>(indices);

	std::array<const char*, 2> texturePaths{
		"assets/textures/container.png",
		"assets/textures/container_specular.png",
	};

	VkDeviceSize uboSize = sizeof(UniformBufferObject);
	VkDeviceSize minAlignment = Device::GetDeviceProperties().limits.minUniformBufferOffsetAlignment;
	m_DUboAlignmentSize = DynamicUniformBufferObject::CalcAlignmentSize(minAlignment);
	uint64_t dUboBufferSize = DynamicUniformBufferObject::CalcBufferSize(numInstances, m_DUboAlignmentSize);

	m_UniformBuffers.reserve(maxFramesInFlight);
	m_DynamicUniformBuffers.reserve(maxFramesInFlight);

	for (uint64_t i = 0; i < maxFramesInFlight; ++i)
	{
		m_UniformBuffers.emplace_back(
			uboSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uboSize);
		m_DynamicUniformBuffers.emplace_back(dUboBufferSize,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_DUboAlignmentSize);
	}

	m_Textures.reserve(texturePaths.size());
	for (const auto& texturePath : texturePaths)
		m_Textures.emplace_back(texturePath);

	std::vector<VkDescriptorBufferInfo> uniformBufferInfos = UniformBuffer::GetBufferInfos(m_UniformBuffers);
	std::vector<VkDescriptorBufferInfo> dynamicUniformBufferInfos =
		UniformBuffer::GetBufferInfos(m_DynamicUniformBuffers);
	std::vector<VkDescriptorImageInfo> textureImageInfos = Texture2D::GetImageInfos(m_Textures);

	m_DescriptorSet = std::make_unique<DescriptorSet>(maxFramesInFlight);
	m_DescriptorSet->SetupLayout({
		DescriptorSet::CreateLayout( //
			DescriptorType::UNIFORM_BUFFER,
			ShaderType::VERTEX,
			0,
			1,
			uniformBufferInfos.data(),
			nullptr), //
		DescriptorSet::CreateLayout( //
			DescriptorType::UNIFORM_BUFFER_DYNAMIC,
			ShaderType::VERTEX,
			1,
			1,
			dynamicUniformBufferInfos.data(),
			nullptr), //
		DescriptorSet::CreateLayout( //
			DescriptorType::SAMPLED_IMAGE,
			ShaderType::FRAGMENT,
			2,
			static_cast<uint32_t>(m_Textures.size()),
			nullptr,
			textureImageInfos.data()), //
		DescriptorSet::CreateLayout( //
			DescriptorType::SAMPLER,
			ShaderType::FRAGMENT,
			3,
			1,
			nullptr,
			&textureImageInfos[0]), // we only need the sampler
	});
	m_DescriptorSet->Create();
	m_Pipeline = std::make_unique<Pipeline>("assets/shaders/phongLighting.vert.spv",
		"assets/shaders/phongLighting.frag.spv",
		m_DescriptorSet->GetPipelineLayout(),
		renderPass);
}

void Cube::Draw(VkCommandBuffer commandBuffer,
	const uint64_t currentFrameIndex,
	const uint32_t dynamicOffsetCount,
	const uint32_t* dynamicOffset)
{
	m_VertexBuffer->Bind(commandBuffer);
	m_IndexBuffer->Bind(commandBuffer);
	m_Pipeline->Bind(commandBuffer);
	m_DescriptorSet->Bind(commandBuffer, currentFrameIndex, dynamicOffsetCount, dynamicOffset);
	m_IndexBuffer->Draw(commandBuffer);
}

void Cube::UpdateUniformBuffers(const UniformBufferObject& ubo,
	const DynamicUniformBufferObject& dUbo,
	const uint32_t currentFrameIndex)
{
	m_UniformBuffers[currentFrameIndex].Map(&ubo);
	m_DynamicUniformBuffers[currentFrameIndex].Map(dUbo.buffer);
}


LightCube::LightCube(VkRenderPass renderPass, const uint32_t maxFramesInFlight, const uint64_t numInstances)
{
	m_VertexBuffer = std::make_unique<VertexBuffer>(vertices);
	m_IndexBuffer = std::make_unique<IndexBuffer>(indices);

	VkDeviceSize uboSize = sizeof(UniformBufferObject);
	m_UniformBuffers.reserve(maxFramesInFlight);

	for (uint64_t i = 0; i < maxFramesInFlight; ++i)
	{
		m_UniformBuffers.emplace_back(
			uboSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uboSize);
	}

	std::vector<VkDescriptorBufferInfo> uniformBufferInfos = UniformBuffer::GetBufferInfos(m_UniformBuffers);

	m_DescriptorSet = std::make_unique<DescriptorSet>(maxFramesInFlight);
	m_DescriptorSet->SetupLayout({
		DescriptorSet::CreateLayout( //
			DescriptorType::UNIFORM_BUFFER,
			ShaderType::VERTEX,
			0,
			1,
			uniformBufferInfos.data(),
			nullptr), //
	});
	m_DescriptorSet->Create();
	m_Pipeline = std::make_unique<Pipeline>("assets/shaders/lightCube.vert.spv",
		"assets/shaders/lightCube.frag.spv",
		m_DescriptorSet->GetPipelineLayout(),
		renderPass);
}

void LightCube::Draw(VkCommandBuffer commandBuffer, const uint64_t currentFrameIndex)
{
	m_VertexBuffer->Bind(commandBuffer);
	m_IndexBuffer->Bind(commandBuffer);
	m_Pipeline->Bind(commandBuffer);
	m_DescriptorSet->Bind(commandBuffer, currentFrameIndex, 0, nullptr);
	m_IndexBuffer->Draw(commandBuffer);
}

void LightCube::UpdateUniformBuffers(const LightCubeUBO& ubo, const uint32_t currentFrameIndex)
{
	m_UniformBuffers[currentFrameIndex].Map(&ubo);
}