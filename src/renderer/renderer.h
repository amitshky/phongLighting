#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "imgui/backends/imgui_impl_vulkan.h"
#include "renderer/vulkanContext.h"
#include "renderer/device.h"
#include "renderer/commandPool.h"
#include "renderer/commandBuffer.h"
#include "renderer/swapchain.h"
#include "renderer/vertexBuffer.h"
#include "renderer/indexBuffer.h"
#include "renderer/descriptor.h"
#include "renderer/uniformBuffer.h"
#include "renderer/texture.h"
#include "renderer/pipeline.h"
#include "renderer/camera.h"
#include "editor/ubo.h"


class Renderer
{
public:
	Renderer(const char* title, const VulkanConfig& config, const std::shared_ptr<Window>& window);
	~Renderer();

	void Draw(float deltatime);
	void OnResize(int width, int height);
	void OnMouseMove(double xpos, double ypos);

	void BeginScene();
	void EndScene();


private:
	void Init(const char* title);
	void Cleanup();

	// synchronization objects
	void CreateSyncObjects();
	void UpdateUniformBuffers(uint32_t currentFrameIndex);

private:
	const VulkanConfig m_Config;
	std::shared_ptr<Window> m_Window;

	std::shared_ptr<VulkanContext> m_VulkanContext{};
	std::shared_ptr<Device> m_Device{};
	std::shared_ptr<CommandPool> m_CommandPool{};

	std::unique_ptr<Swapchain> m_Swapchain{};

	std::unique_ptr<VertexBuffer> m_VertexBuffer{};
	std::unique_ptr<IndexBuffer> m_IndexBuffer{};

	UniformBufferObject m_Ubo{};
	DynamicUniformBufferObject m_DUbo{};
	std::vector<UniformBuffer> m_UniformBuffers{};
	std::vector<UniformBuffer> m_DynamicUniformBuffers{};
	std::unique_ptr<Texture2D> m_Texture{};
	std::unique_ptr<DescriptorSet> m_DescriptorSet{};

	std::unique_ptr<Pipeline> m_Pipeline{};
	std::unique_ptr<CommandBuffer> m_CommandBuffer{};

	// synchronization objects
	// used to acquire swapchain images
	std::vector<VkSemaphore> m_ImageAvailableSemaphores{};
	// signaled when command buffers have finished execution
	std::vector<VkSemaphore> m_RenderFinishedSemaphores{};
	std::vector<VkFence> m_InFlightFences{};

	std::unique_ptr<Camera> m_Camera{};

	VkCommandBuffer m_ActiveCommandBuffer{};
	uint32_t m_CurrentFrameIndex = 0;
	uint32_t m_NextFrameIndex = 0; // acquired from swapchain
	bool m_FramebufferResized = false;
};