#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "renderer/vulkanContext.h"
#include "renderer/device.h"


class Renderer
{
public:
	Renderer(const char* title, const VulkanConfig& config, const std::shared_ptr<Window>& window);
	~Renderer();

private:
	void Init(const char* title, const VulkanConfig& config);

	// swapchain
	VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateSwapchain();
	void CreateSwapchainImageViews();
	void CleanupSwapchain();

	// render pass
	void CreateRenderPass();
	VkFormat FindDepthFormat();
	// framebuffer
	void CreateColorResource();
	void CreateDepthResource();
	void CreateFramebuffers();

	// pipeline
	void CreateGraphicsPipeline();

private:
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
};