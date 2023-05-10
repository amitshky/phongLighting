#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "renderer/vulkanContext.h"
#include "renderer/device.h"


class Renderer
{
public:
	Renderer(const char* title, const VulkanConfig& config, std::shared_ptr<Window> window);
	~Renderer();

	void Draw();
	void OnResize();

private:
	void Init(const char* title, const VulkanConfig& config);
	void Terminate();

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
	void CreateCommandPool();
	void CreateCommandBuffers();
	void RecordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	// synchronization objects
	void CreateSyncObjects();

private:
	const VulkanConfig m_Config;
	std::shared_ptr<Window> m_Window;

	VulkanContext* m_VulkanContext;
	Device* m_Device;

	// swapchain
	VkSwapchainKHR m_Swapchain;
	std::vector<VkImage> m_SwapchainImages;
	VkFormat m_SwapchainImageFormat;
	VkExtent2D m_SwapchainExtent;
	std::vector<VkImageView> m_SwapchainImageViews;

	// render pass
	VkRenderPass m_RenderPass;

	// framebuffer
	VkImage m_ColorImage;
	VkDeviceMemory m_ColorImageMemory;
	VkImageView m_ColorImageView;
	VkImage m_DepthImage;
	VkDeviceMemory m_DepthImageMemory;
	VkImageView m_DepthImageView;
	std::vector<VkFramebuffer> m_SwapchainFramebuffers;

	// pipeline
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_Pipeline;

	// command buffer
	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	// synchronization objects
	// used to acquire swapchain images
	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	// signaled when command buffers have finished execution
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;

	uint32_t m_CurrentFrameIndex = 0;
	bool m_FramebufferResized = false;
};