#pragma once

#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "renderer/vertexBuffer.h"
#include "renderer/indexBuffer.h"
#include "renderer/texture.h"
#include "renderer/uniformBuffer.h"
#include "renderer/descriptor.h"
#include "renderer/pipeline.h"
#include "editor/ubo.h"


class Mesh
{
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	void Init();
	void Draw(VkCommandBuffer commandBuffer) const;

private:
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	std::unique_ptr<VertexBuffer> m_VertexBuffer{};
	std::unique_ptr<IndexBuffer> m_IndexBuffer{};
};

class Model
{
public:
	Model(const char* path,
		VkRenderPass renderPass,
		const uint32_t maxFramesInFlight,
		const uint64_t numInstances,
		bool flipUVs = false);

	void Draw(VkCommandBuffer commandBuffer,
		const uint64_t currentFrameIndex,
		const uint32_t dynamicOffsetCount,
		const uint32_t* dynamicOffset);
	void UpdateUniformBuffers(const UniformBufferObject& ubo,
		const DynamicUniformBufferObject& dUbo,
		const uint32_t currentFrameIndex);

private:
	void LoadModel(const std::string& path, bool flipUVs);
	void SetupRenderingResources();

	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	void LoadTextures(aiMaterial* material, aiTextureType type);

private:
	VkRenderPass m_RenderPass;
	const uint32_t m_MaxFramesInFlight;
	const uint64_t m_NumInstances;

	std::string m_Directory;
	std::vector<Mesh> m_Meshes;
	std::vector<std::shared_ptr<Texture2D>> m_LoadedTextures{};

	uint64_t m_DUboAlignmentSize = 0;
	std::vector<UniformBuffer> m_UniformBuffers{};
	std::vector<UniformBuffer> m_DynamicUniformBuffers{};
	std::unique_ptr<DescriptorSet> m_DescriptorSet{};
	std::unique_ptr<Pipeline> m_Pipeline{};
};