#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "renderer/vulkanContext.h"
#include "renderer/device.h"
#include "renderer/commandPool.h"
#include "renderer/vertexBuffer.h"
#include "renderer/indexBuffer.h"
#include "renderer/descriptor.h"
#include "renderer/uniformBuffer.h"
#include "renderer/texture.h"
#include "renderer/camera.h"
#include "editor/ubo.h"


class Renderer
{
public:
	Renderer(const char* title, const VulkanConfig& config, std::shared_ptr<Window> window);
	~Renderer();

	void Draw(float deltatime);
	void OnResize(int width, int height);
	void OnMouseMove(double xpos, double ypos);

private:
	void Init(const char* title);
	void Cleanup();

	// swapchain
	static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateSwapchain();
	void CreateSwapchainImageViews();
	void CleanupSwapchain();
	void RecreateSwapchain();

	// render pass
	void CreateRenderPass();
	static VkFormat FindDepthFormat();
	// framebuffer
	void CreateColorResource();
	void CreateDepthResource();
	void CreateFramebuffers();

	// pipeline
	void CreateGraphicsPipeline();
	// command buffer
	void CreateCommandBuffers();
	void RecordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	// synchronization objects
	void CreateSyncObjects();

	void UpdateUniformBuffers(uint32_t currentFrameIndex);

private:
	const VulkanConfig m_Config;
	std::shared_ptr<Window> m_Window;

	VulkanContext* m_VulkanContext = nullptr;
	Device* m_Device = nullptr;
	CommandPool* m_CommandPool = nullptr;

	// swapchain
	VkSwapchainKHR m_Swapchain{};
	std::vector<VkImage> m_SwapchainImages{};
	VkFormat m_SwapchainImageFormat{};
	VkExtent2D m_SwapchainExtent{};
	std::vector<VkImageView> m_SwapchainImageViews{};

	// render pass
	VkRenderPass m_RenderPass{};
	// framebuffer
	VkImage m_ColorImage{};
	VkDeviceMemory m_ColorImageMemory{};
	VkImageView m_ColorImageView{};
	VkImage m_DepthImage{};
	VkDeviceMemory m_DepthImageMemory{};
	VkImageView m_DepthImageView{};
	std::vector<VkFramebuffer> m_SwapchainFramebuffers{};

	std::unique_ptr<VertexBuffer> m_VertexBuffer{};
	std::unique_ptr<IndexBuffer> m_IndexBuffer{};

	UniformBufferObject m_Ubo{};
	DynamicUniformBufferObject m_DUbo{};
	std::vector<UniformBuffer> m_UniformBuffers{};
	std::vector<UniformBuffer> m_DynamicUniformBuffers{};
	std::unique_ptr<Texture2D> m_Texture{};
	std::unique_ptr<DescriptorSet> m_DescriptorSet{};

	// pipeline
	VkPipeline m_Pipeline{};
	// command buffer
	std::vector<VkCommandBuffer> m_CommandBuffers{};
	// synchronization objects
	// used to acquire swapchain images
	std::vector<VkSemaphore> m_ImageAvailableSemaphores{};
	// signaled when command buffers have finished execution
	std::vector<VkSemaphore> m_RenderFinishedSemaphores{};
	std::vector<VkFence> m_InFlightFences{};

	std::unique_ptr<Camera> m_Camera{};

	uint32_t m_CurrentFrameIndex = 0;
	bool m_FramebufferResized = false;
};