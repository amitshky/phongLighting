#include "renderer/model.h"

#include "core/core.h"
#include "glm/glm.hpp"
#include "renderer/device.h"


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: m_Vertices{ vertices },
	  m_Indices{ indices }
{
	Init();
}

void Mesh::Init()
{
	m_VertexBuffer = std::make_unique<VertexBuffer>(m_Vertices);
	m_IndexBuffer = std::make_unique<IndexBuffer>(m_Indices);
}

void Mesh::Draw(VkCommandBuffer commandBuffer) const
{
	m_VertexBuffer->Bind(commandBuffer);
	m_IndexBuffer->Bind(commandBuffer);
	m_IndexBuffer->Draw(commandBuffer);
}

Model::Model(const char* path,
	VkRenderPass renderPass,
	const uint32_t maxFramesInFlight,
	const uint64_t numInstances,
	bool flipUVs)
	: m_RenderPass{ renderPass },
	  m_MaxFramesInFlight{ maxFramesInFlight },
	  m_NumInstances{ numInstances }
{
	Logger::Info("Loading model...");
	LoadModel(path, flipUVs);
	SetupRenderingResources();
	Logger::Info("Model loaded");
}

void Model::LoadModel(const std::string& path, bool flipUVs)
{
	uint32_t pFlags = aiProcess_Triangulate;
	if (flipUVs)
		pFlags |= aiProcess_FlipUVs;

	Assimp::Importer importer{};
	const aiScene* scene = importer.ReadFile(path, pFlags);
	THROW(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode,
		"Failed to load model! {}",
		importer.GetErrorString())

	m_Directory = path.substr(0, path.find_last_of('/'));
	m_Meshes.reserve(static_cast<uint64_t>(scene->mRootNode->mNumMeshes));

	ProcessNode(scene->mRootNode, scene);
}

void Model::SetupRenderingResources()
{
	VkDeviceSize uboSize = sizeof(UniformBufferObject);
	VkDeviceSize minAlignment = Device::GetDeviceProperties().limits.minUniformBufferOffsetAlignment;
	m_DUboAlignmentSize = DynamicUniformBufferObject::CalcAlignmentSize(minAlignment);
	uint64_t dUboBufferSize = DynamicUniformBufferObject::CalcBufferSize(m_NumInstances, m_DUboAlignmentSize);

	m_UniformBuffers.reserve(m_MaxFramesInFlight);
	m_DynamicUniformBuffers.reserve(m_MaxFramesInFlight);

	for (uint64_t i = 0; i < m_MaxFramesInFlight; ++i)
	{
		m_UniformBuffers.emplace_back(
			uboSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uboSize);
		m_DynamicUniformBuffers.emplace_back(dUboBufferSize,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_DUboAlignmentSize);
	}

	std::vector<VkDescriptorBufferInfo> uniformBufferInfos = UniformBuffer::GetBufferInfos(m_UniformBuffers);
	std::vector<VkDescriptorBufferInfo> dynamicUniformBufferInfos =
		UniformBuffer::GetBufferInfos(m_DynamicUniformBuffers);
	std::vector<VkDescriptorImageInfo> textureImageInfos = Texture2D::GetImageInfos(m_LoadedTextures);

	m_DescriptorSet = std::make_unique<DescriptorSet>(m_MaxFramesInFlight);
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
			static_cast<uint32_t>(m_LoadedTextures.size()),
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
		m_RenderPass);
}

void Model::Draw(VkCommandBuffer commandBuffer,
	const uint64_t currentFrameIndex,
	const uint32_t dynamicOffsetCount,
	const uint32_t* dynamicOffset)
{
	m_Pipeline->Bind(commandBuffer);
	m_DescriptorSet->Bind(commandBuffer, currentFrameIndex, dynamicOffsetCount, dynamicOffset);

	for (const auto& mesh : m_Meshes)
		mesh.Draw(commandBuffer);
}

void Model::UpdateUniformBuffers(const UniformBufferObject& ubo,
	const DynamicUniformBufferObject& dUbo,
	const uint32_t currentFrameIndex)
{
	m_UniformBuffers[currentFrameIndex].Map(&ubo);
	m_DynamicUniformBuffers[currentFrameIndex].Map(dUbo.buffer);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_Meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices{};
	vertices.reserve(static_cast<uint64_t>(mesh->mNumVertices));
	std::vector<uint32_t> indices{};
	std::vector<Texture2D> textures{};

	// process vertices
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex{};
		glm::vec3 position{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		vertex.pos = position;

		if (mesh->HasNormals())
		{
			glm::vec3 normal{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			vertex.normal = normal;
		}
		else
			vertex.normal = glm::vec3(0.0f);

		if (mesh->mTextureCoords[0]) // check if the mesh has texture coords
		{
			glm::vec2 texCoords{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			vertex.texCoord = texCoords;
		}
		else
			vertex.texCoord = glm::vec2(0.0f);

		vertices.push_back(vertex);
	}

	// process indices
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; ++j)
			indices.push_back(face.mIndices[j]);
	}

	// process materials
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		LoadTextures(material, aiTextureType_DIFFUSE);
		LoadTextures(material, aiTextureType_SPECULAR);
	}

	return Mesh{ vertices, indices };
}

void Model::LoadTextures(aiMaterial* material, aiTextureType type)
{
	uint32_t textureCount = material->GetTextureCount(type);
	if (textureCount == 0)
	{
		// fallback texture if the texture could not be loaded
		const char* texPath = "assets/textures/checkerboard.png";
		Logger::Info("    Fallback texture loaded: \"{}\"", texPath);
		m_LoadedTextures.push_back(std::make_shared<Texture2D>(texPath));
		return;
	}

	for (uint32_t i = 0; i < textureCount; ++i)
	{
		aiString filename;
		bool skip = false;

		material->GetTexture(type, i, &filename);
		std::string texturePath = m_Directory + '/' + filename.C_Str();

		// check if the texture has already been loaded
		for (const auto& texture : m_LoadedTextures)
		{
			if (texturePath == texture->GetPath())
			{
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			m_LoadedTextures.push_back(std::make_shared<Texture2D>(texturePath.c_str()));
			Logger::Info("    Loaded texture: \"{}\"", texturePath.c_str());
		}
	}
}
