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
#include "renderer/model.h"
#include "editor/ubo.h"
#include "editor/objects.h"


class Renderer
{
public:
	Renderer(const char* title, const VulkanConfig& config, const std::shared_ptr<Window>& window);
	~Renderer();

	void Draw(float deltatime, uint32_t fpsCount);
	void OnResize(int width, int height);
	void OnMouseMove(double xpos, double ypos);

	void BeginScene();
	void EndScene();

private:
	void Init(const char* title);
	void Cleanup();

	void CreateSyncObjects();
	void UpdateUniformBuffers(uint32_t currentFrameIndex);
	void OnUIRender(uint32_t fpsCount);

private:
	const VulkanConfig m_Config;
	std::shared_ptr<Window> m_Window;

	std::shared_ptr<VulkanContext> m_VulkanContext{};
	std::shared_ptr<Device> m_Device{};
	std::shared_ptr<CommandPool> m_CommandPool{};

	std::unique_ptr<Swapchain> m_Swapchain{};

	std::unique_ptr<Model> m_BackpackModel{};
	std::unique_ptr<Model> m_CerberusModel{};
	std::unique_ptr<Cube> m_Cube{};
	std::unique_ptr<LightCube> m_LightCube{};

	UniformBufferObject m_Ubo{};
	DynamicUniformBufferObject m_DUbo{};
	LightCubeUBO m_LightCubeUbo{};

	// uniform values to be displayed in the ui
	glm::vec3 m_BackpackPos{ -1.0f, 0.0f, 0.0f };
	float m_BackpackRotateX{ 0.0f };
	float m_BackpackRotateY{ 0.0f };
	float m_BackpackRotateZ{ 0.0f };
	glm::vec3 m_CerberusPos{ 0.0f, 0.0f, 0.0f };
	float m_CerberusRotateX{ -90.0f };
	float m_CerberusRotateY{ 0.0f };
	float m_CerberusRotateZ{ 180.0f };
	glm::vec3 m_CubePos{ 1.0f, 0.0f, 0.0f };
	float m_CubeRotateX{ 0.0f };
	float m_CubeRotateY{ 0.0f };
	float m_CubeRotateZ{ 0.0f };

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