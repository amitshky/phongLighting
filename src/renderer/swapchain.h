#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include "core/window.h"


class Swapchain
{
public:
	Swapchain(const std::shared_ptr<Window>& window);
	~Swapchain();

	void Cleanup();
	void RecreateSwapchain();
	VkResult AcquireNextImageIndex(VkSemaphore imageAvailableSemaphore, uint32_t* nextImageIndex);

	void BeginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void EndRenderPass(VkCommandBuffer commandBuffer);

	inline VkSwapchainKHR GetHandle() const { return m_Swapchain; }
	inline VkRenderPass GetRenderPass() const { return m_RenderPass; }
	inline uint32_t GetWidth() const { return m_SwapchainExtent.width; }
	inline uint32_t GetHeight() const { return m_SwapchainExtent.height; }

private:
	void Init();

	void CreateSwapchain();
	void CreateSwapchainImageViews();

	void CreateRenderPass();
	void CreateColorResource();
	void CreateDepthResource();
	void CreateFramebuffers();

	std::array<VkClearValue, 3> ClearAttachmentValues();
	void SetViewport(VkCommandBuffer commandBuffer);
	void SetScissor(VkCommandBuffer commandBuffer);

	static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	static VkFormat FindDepthFormat();

private:
	std::shared_ptr<Window> m_Window;

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
};