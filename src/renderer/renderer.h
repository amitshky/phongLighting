#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "renderer/vulkanContext.h"
#include "renderer/device.h"
#include "renderer/vertexBuffer.h"
#include "renderer/uniformBuffer.h"
#include "renderer/camera.h"


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

	// vertex and index buffer
	void CreateVertexBuffer();
	void CreateIndexBuffer();

	// uniform buffer
	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateUniformBuffers();
	void CreateDescriptorSets();
	void UpdateUniformBuffer(uint32_t currentFrameIndex);

	// textures
	void CreateTextureImage();
	void CreateTextureImageView();
	void CreateTextureSampler();
	void TransitionImageLayout(VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		uint32_t miplevels);

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

	// vertex buffer
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	// index buffer
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;

	// resource descriptors
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkPipelineLayout m_PipelineLayout;
	VkDescriptorPool m_DescriptorPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;
	// uniform buffers
	std::vector<VkBuffer> m_UniformBuffers;
	std::vector<VkDeviceMemory> m_UniformBufferMemory;
	std::vector<void*> m_UniformBufferMapped;
	std::vector<VkBuffer> m_DynamicUniformBuffers;
	std::vector<VkDeviceMemory> m_DynamicUniformBufferMemory;
	std::vector<void*> m_DynamicUniformBufferMapped;
	UniformBufferObject m_Ubo{};
	DynamicUniformBufferObject m_DUbo{};

	// textures
	VkImage m_TextureImage;
	VkDeviceMemory m_TextureImageMemory;
	VkImageView m_TextureImageView;
	VkSampler m_TextureSampler;

	std::unique_ptr<Camera> m_Camera;

	uint32_t m_CurrentFrameIndex = 0;
	bool m_FramebufferResized = false;
	uint32_t m_Miplevels = 1;
};