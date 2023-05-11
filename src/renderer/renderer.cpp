#include "renderer/renderer.h"

#include <array>
#include <numeric>
#include <chrono>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/core.h"
#include "renderer/shader.h"
#include "utils/utils.h"


const std::vector<Vertex> vertices{
	{{ -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }},
	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }},
	{ { 0.5f, 0.5f },  { 0.0f, 0.0f, 1.0f }},
	{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }}
};

const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

Renderer::Renderer(const char* title, const VulkanConfig& config, std::shared_ptr<Window> window)
	: m_Config{ config },
	  m_Window{ std::move(window) }
{
	Init(title);
}

Renderer::~Renderer()
{
	Terminate();
}

void Renderer::Init(const char* title)
{
	m_VulkanContext = VulkanContext::Create(title, m_Config, m_Window);
	m_Device = Device::Create(m_Config, m_Window->GetWindowSurface());

	CreateSwapchain();
	CreateSwapchainImageViews();

	CreateRenderPass();
	CreateColorResource();
	CreateDepthResource();
	CreateFramebuffers();

	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();

	CreateCommandPool();
	CreateCommandBuffers();

	CreateSyncObjects();

	CreateVertexBuffer();
	CreateIndexBuffer();

	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
}

void Renderer::Terminate()
{
	Device::WaitIdle();

	vkDestroyDescriptorPool(Device::GetDevice(), m_DescriptorPool, nullptr);
	for (uint32_t i = 0; i < m_Config.maxFramesInFlight; ++i)
	{
		vkUnmapMemory(Device::GetDevice(), m_UniformBufferMemory[i]);
		vkFreeMemory(Device::GetDevice(), m_UniformBufferMemory[i], nullptr);
		vkDestroyBuffer(Device::GetDevice(), m_UniformBuffers[i], nullptr);
	}
	vkDestroyDescriptorSetLayout(Device::GetDevice(), m_DescriptorSetLayout, nullptr);

	vkFreeMemory(Device::GetDevice(), m_IndexBufferMemory, nullptr);
	vkDestroyBuffer(Device::GetDevice(), m_IndexBuffer, nullptr);

	vkFreeMemory(Device::GetDevice(), m_VertexBufferMemory, nullptr);
	vkDestroyBuffer(Device::GetDevice(), m_VertexBuffer, nullptr);

	for (size_t i = 0; i < m_Config.maxFramesInFlight; ++i)
	{
		vkDestroySemaphore(Device::GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(Device::GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(Device::GetDevice(), m_InFlightFences[i], nullptr);
	}

	// command buffers are automatically destroyed
	vkDestroyCommandPool(Device::GetDevice(), m_CommandPool, nullptr);

	vkDestroyPipeline(Device::GetDevice(), m_Pipeline, nullptr);
	vkDestroyPipelineLayout(Device::GetDevice(), m_PipelineLayout, nullptr);

	CleanupSwapchain();
	vkDestroyRenderPass(Device::GetDevice(), m_RenderPass, nullptr);

	delete m_Device;
	delete m_VulkanContext;
}

void Renderer::Draw()
{
	// wait for previous frame to signal the fence
	vkWaitForFences(Device::GetDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);

	// acquire image from the swapchain
	// signals the semaphore
	uint32_t nextImageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(Device::GetDevice(),
		m_Swapchain,
		UINT64_MAX,
		m_ImageAvailableSemaphores[m_CurrentFrameIndex],
		VK_NULL_HANDLE,
		&nextImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// we cannot simply return here, because the queue is never
		// submitted and thus the fences are never signaled , causing a
		// deadlock; to solve this we delay resetting the fences until
		// after we check the swapchain
		RecreateSwapchain();
		return;
	}

	THROW(result != VK_SUCCESS, "Failed to acquire swapchain image!")

	// resetting the fence has been set after the result has been checked to
	// avoid a deadlock reset the fence to unsignaled state
	vkResetFences(Device::GetDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex]);

	// record command buffer
	vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrameIndex], 0);
	RecordCommandBuffers(m_CommandBuffers[m_CurrentFrameIndex], nextImageIndex);

	UpdateUniformBuffer(m_CurrentFrameIndex);

	// submit the command buffer
	std::array<VkSemaphore, 1> waitSemaphores{ m_ImageAvailableSemaphores[m_CurrentFrameIndex] };
	std::array<VkSemaphore, 1> signalSemaphores{ m_RenderFinishedSemaphores[m_CurrentFrameIndex] };
	std::array<VkPipelineStageFlags, 1> waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrameIndex];
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	// signals the fence after executing the command buffer
	THROW(
		vkQueueSubmit(Device::GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrameIndex]) != VK_SUCCESS,
		"Failed to submit draw command buffer!")

	// present the image
	std::array<VkSwapchainKHR, 1> swapchains{ m_Swapchain };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	presentInfo.pWaitSemaphores = signalSemaphores.data();
	presentInfo.swapchainCount = static_cast<uint32_t>(swapchains.size());
	presentInfo.pSwapchains = swapchains.data();
	presentInfo.pImageIndices = &nextImageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(Device::GetPresentQueue(), &presentInfo);

	// update current frame index
	m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_Config.maxFramesInFlight;
}

void Renderer::OnResize()
{
	RecreateSwapchain();
}

void Renderer::CreateSwapchain()
{
	SwapchainSupportDetails swapchainSupport =
		Device::QuerySwapchainSupport(Device::GetPhysicalDevice(), VulkanContext::GetWindowSurface());
	VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = ChoosePresentMode(swapchainSupport.presentModes);
	VkExtent2D extent = ChooseExtent(swapchainSupport.capabilities);

	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
		imageCount = swapchainSupport.capabilities.maxImageCount;

	QueueFamilyIndices queueFamilyIndices =
		Device::FindQueueFamilies(Device::GetPhysicalDevice(), VulkanContext::GetWindowSurface());

	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = VulkanContext::GetWindowSurface();
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value())
	{
		uint32_t indicesArr[]{ queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = indicesArr;
	}
	else
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	swapchainInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

	THROW(vkCreateSwapchainKHR(Device::GetDevice(), &swapchainInfo, nullptr, &m_Swapchain) != VK_SUCCESS,
		"Failed to create swapchain!")

	vkGetSwapchainImagesKHR(Device::GetDevice(), m_Swapchain, &imageCount, nullptr);
	m_SwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(Device::GetDevice(), m_Swapchain, &imageCount, m_SwapchainImages.data());

	m_SwapchainImageFormat = surfaceFormat.format;
	m_SwapchainExtent = extent;
}

void Renderer::CreateSwapchainImageViews()
{
	m_SwapchainImageViews.resize(m_SwapchainImages.size());

	for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
	{
		m_SwapchainImageViews[i] =
			utils::CreateImageView(m_SwapchainImages[i], m_SwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void Renderer::CleanupSwapchain()
{
	vkDestroyImageView(Device::GetDevice(), m_DepthImageView, nullptr);
	vkDestroyImage(Device::GetDevice(), m_DepthImage, nullptr);
	vkFreeMemory(Device::GetDevice(), m_DepthImageMemory, nullptr);

	vkDestroyImageView(Device::GetDevice(), m_ColorImageView, nullptr);
	vkDestroyImage(Device::GetDevice(), m_ColorImage, nullptr);
	vkFreeMemory(Device::GetDevice(), m_ColorImageMemory, nullptr);

	for (const auto& framebuffer : m_SwapchainFramebuffers)
		vkDestroyFramebuffer(Device::GetDevice(), framebuffer, nullptr);

	for (const auto& imageView : m_SwapchainImageViews)
		vkDestroyImageView(Device::GetDevice(), imageView, nullptr);

	// swapchain images are destroyed with `vkDestroySwapchainKHR()`
	vkDestroySwapchainKHR(Device::GetDevice(), m_Swapchain, nullptr);
}

void Renderer::RecreateSwapchain()
{
	while (m_Window->IsMinimized())
	{
		m_Window->WaitEvents();
	}

	Device::WaitIdle();
	CleanupSwapchain();

	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateColorResource();
	CreateDepthResource();
	CreateFramebuffers();
}

VkSurfaceFormatKHR Renderer::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}

	return availableFormats[0];
}

VkPresentModeKHR Renderer::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		int width = 0;
		int height = 0;
		m_Window->GetFramebufferSize(&width, &height);

		VkExtent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		// clamp the values to the allowed range
		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height =
			std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return extent;
	}
}

void Renderer::CreateRenderPass()
{
	// color attachment description
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_SwapchainImageFormat;
	colorAttachment.samples = Device::GetMSAASamplesCount();
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// depth attachment description
	VkFormat depthFormat = FindDepthFormat();
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = depthFormat;
	depthAttachment.samples = Device::GetMSAASamplesCount();
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// color resolve attachment description (Multisample)
	VkAttachmentDescription colorResolveAttachment{};
	colorResolveAttachment.format = m_SwapchainImageFormat;
	colorResolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // sample count after MSAA
	colorResolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorResolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorResolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorResolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorResolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorResolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// attachment refrences
	VkAttachmentReference colorRef{};
	colorRef.attachment = 0; // attachment index
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef{};
	depthRef.attachment = 1;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorResolveRef{};
	colorResolveRef.attachment = 2;
	colorResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// subpass
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pResolveAttachments = &colorResolveRef;
	subpass.pDepthStencilAttachment = &depthRef;

	// subpass dependency
	VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 3> attachments{ colorAttachment, depthAttachment, colorResolveAttachment };

	// render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;

	THROW(vkCreateRenderPass(Device::GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS,
		"Failed to create render pass!");
}

VkFormat Renderer::FindDepthFormat()
{
	return Device::FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void Renderer::CreateColorResource()
{
	VkFormat colorFormat = m_SwapchainImageFormat;
	uint32_t miplevels = 1;

	utils::CreateImage(m_SwapchainExtent.width,
		m_SwapchainExtent.height,
		miplevels,
		Device::GetMSAASamplesCount(),
		colorFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_ColorImage,
		m_ColorImageMemory);

	m_ColorImageView = utils::CreateImageView(m_ColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, miplevels);
}

void Renderer::CreateDepthResource()
{
	VkFormat depthFormat = FindDepthFormat();
	uint32_t miplevels = 1;

	utils::CreateImage(m_SwapchainExtent.width,
		m_SwapchainExtent.height,
		miplevels,
		Device::GetMSAASamplesCount(),
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_DepthImage,
		m_DepthImageMemory);

	m_DepthImageView = utils::CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, miplevels);
}

void Renderer::CreateFramebuffers()
{
	m_SwapchainFramebuffers.resize(m_SwapchainImages.size());

	for (size_t i = 0; i < m_SwapchainImages.size(); ++i)
	{
		std::array<VkImageView, 3> fbAttachments{ m_ColorImageView, m_DepthImageView, m_SwapchainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
		framebufferInfo.pAttachments = fbAttachments.data();
		framebufferInfo.width = m_SwapchainExtent.width;
		framebufferInfo.height = m_SwapchainExtent.height;
		framebufferInfo.layers = 1;

		THROW(vkCreateFramebuffer(Device::GetDevice(), &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i])
				  != VK_SUCCESS,
			"Failed to create framebuffer!")
	}
}

void Renderer::CreateGraphicsPipeline()
{
	// shader stages
	Shader vertexShader{ "assets/shaders/helloTriangle.vert.spv", ShaderType::VERTEX };
	Shader fragmentShader{ "assets/shaders/helloTriangle.frag.spv", ShaderType::FRAGMENT };
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{ vertexShader.GetShaderStage(),
		fragmentShader.GetShaderStage() };

	// fixed functions
	// vertex input
	auto vertexBindingDesc = Vertex::GetBindingDescription();
	auto vertexAttrDesc = Vertex::GetAttributeDescription();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttrDesc.size());
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc.data();

	// input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	// viewport state
	VkPipelineViewportStateCreateInfo viewportStateInfo{};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.scissorCount = 1;

	// rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizationStateInfo{};
	rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateInfo.lineWidth = 1.0f;
	rasterizationStateInfo.cullMode = VK_CULL_MODE_NONE;
	// we specify counter clockwise because in the projection matrix we flipped the y-coord
	rasterizationStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	// the depth value can be altered by adding a constant value based on fragment slope
	rasterizationStateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateInfo.depthBiasConstantFactor = 0.0f;
	rasterizationStateInfo.depthBiasClamp = 0.0f;
	rasterizationStateInfo.depthBiasSlopeFactor = 0.0f;

	// multisampling
	VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
	multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateInfo.rasterizationSamples = Device::GetMSAASamplesCount();
	multisampleStateInfo.sampleShadingEnable = VK_TRUE;
	multisampleStateInfo.minSampleShading = 0.2f; // min fraction for sample shading; closer to 1 is smoother
	multisampleStateInfo.pSampleMask = nullptr;
	multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleStateInfo.alphaToOneEnable = VK_FALSE;

	// depth and stencil testing
	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo{};
	depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateInfo.minDepthBounds = 0.0f;
	depthStencilStateInfo.maxDepthBounds = 1.0f;
	depthStencilStateInfo.stencilTestEnable = VK_FALSE;
	depthStencilStateInfo.front = {};
	depthStencilStateInfo.back = {};

	// color blending
	// configuration per color attachment
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	// configuration for global color blending settings
	VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
	colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendStateInfo.attachmentCount = 1;
	colorBlendStateInfo.pAttachments = &colorBlendAttachment;
	colorBlendStateInfo.blendConstants[0] = 0.0f;
	colorBlendStateInfo.blendConstants[1] = 0.0f;
	colorBlendStateInfo.blendConstants[2] = 0.0f;
	colorBlendStateInfo.blendConstants[3] = 0.0f;

	// dynamic states
	// allows you to specify the data at drawing time
	std::array<VkDynamicState, 2> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateInfo.pDynamicStates = dynamicStates.data();

	// pipeline layout
	// specify uniforms
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
	// push constants are another way of passing dynamic values to the shaders
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	THROW(vkCreatePipelineLayout(Device::GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS,
		"Failed to create pipeline layout!");

	// graphics pipeline
	VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
	graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// config all previos objects
	graphicsPipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	graphicsPipelineInfo.pStages = shaderStages.data();
	graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
	graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	graphicsPipelineInfo.pViewportState = &viewportStateInfo;
	graphicsPipelineInfo.pRasterizationState = &rasterizationStateInfo;
	graphicsPipelineInfo.pMultisampleState = &multisampleStateInfo;
	graphicsPipelineInfo.pDepthStencilState = &depthStencilStateInfo;
	graphicsPipelineInfo.pColorBlendState = &colorBlendStateInfo;
	graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;
	graphicsPipelineInfo.layout = m_PipelineLayout;
	graphicsPipelineInfo.renderPass = m_RenderPass;
	graphicsPipelineInfo.subpass = 0; // index of subpass
	graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineInfo.basePipelineIndex = -1;

	THROW(vkCreateGraphicsPipelines(Device::GetDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &m_Pipeline)
			  != VK_SUCCESS,
		"Failed to create graphics pipeline!");
}

void Renderer::CreateCommandPool()
{
	QueueFamilyIndices queueIndices =
		Device::FindQueueFamilies(Device::GetPhysicalDevice(), VulkanContext::GetWindowSurface());

	VkCommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = queueIndices.graphicsFamily.value();

	THROW(vkCreateCommandPool(Device::GetDevice(), &commandPoolInfo, nullptr, &m_CommandPool) != VK_SUCCESS,
		"Failed to create command pool!")
}

void Renderer::CreateCommandBuffers()
{
	m_CommandBuffers.resize(m_Config.maxFramesInFlight);

	VkCommandBufferAllocateInfo cmdBuffAllocInfo{};
	cmdBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBuffAllocInfo.commandPool = m_CommandPool;
	cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBuffAllocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	THROW(vkAllocateCommandBuffers(Device::GetDevice(), &cmdBuffAllocInfo, m_CommandBuffers.data()) != VK_SUCCESS,
		"Failed to allocate command buffers!")
}

void Renderer::RecordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo cmdBuffBeginInfo{};
	cmdBuffBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	THROW(vkBeginCommandBuffer(commandBuffer, &cmdBuffBeginInfo) != VK_SUCCESS,
		"Failed to begin recording command buffer!")

	// clear values for each attachment
	std::array<VkClearValue, 3> clearValues;
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	clearValues[2].color = clearValues[0].color;

	// start a render pass
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.framebuffer = m_SwapchainFramebuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_SwapchainExtent;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapchainExtent.width);
	viewport.height = static_cast<float>(m_SwapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapchainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	VkBuffer vertexBuffers[]{ m_VertexBuffer };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_PipelineLayout,
		0,
		1,
		&m_DescriptorSets[m_CurrentFrameIndex],
		0,
		nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
	THROW(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS, "Failed to record command buffer!");
}

void Renderer::CreateSyncObjects()
{
	m_ImageAvailableSemaphores.resize(m_Config.maxFramesInFlight);
	m_RenderFinishedSemaphores.resize(m_Config.maxFramesInFlight);
	m_InFlightFences.resize(m_Config.maxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// create the fence in signaled state so that the first frame doesnt have to wait
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < m_Config.maxFramesInFlight; ++i)
	{
		THROW(vkCreateSemaphore(Device::GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i])
					  != VK_SUCCESS
				  || vkCreateSemaphore(Device::GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i])
						 != VK_SUCCESS
				  || vkCreateFence(Device::GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS,
			"Failed to create synchronization objects!")
	}
}

void Renderer::CreateVertexBuffer()
{
	VkDeviceSize size = sizeof(vertices[0]) * static_cast<uint64_t>(vertices.size());

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	utils::CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMem);

	void* data;
	vkMapMemory(Device::GetDevice(), stagingBufferMem, 0, size, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(size));
	vkUnmapMemory(Device::GetDevice(), stagingBufferMem);

	// create the actual vertex buffer
	utils::CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_VertexBuffer,
		m_VertexBufferMemory);

	utils::CopyBuffer(m_CommandPool, stagingBuffer, m_VertexBuffer, size);

	vkFreeMemory(Device::GetDevice(), stagingBufferMem, nullptr);
	vkDestroyBuffer(Device::GetDevice(), stagingBuffer, nullptr);
}

void Renderer::CreateIndexBuffer()
{
	VkDeviceSize size = sizeof(indices[0]) * static_cast<uint64_t>(indices.size());

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	utils::CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMem);

	void* data;
	vkMapMemory(Device::GetDevice(), stagingBufferMem, 0, size, 0, &data);
	memcpy(data, indices.data(), static_cast<size_t>(size));
	vkUnmapMemory(Device::GetDevice(), stagingBufferMem);

	// create the actual vertex buffer
	utils::CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_IndexBuffer,
		m_IndexBufferMemory);

	utils::CopyBuffer(m_CommandPool, stagingBuffer, m_IndexBuffer, size);

	vkFreeMemory(Device::GetDevice(), stagingBufferMem, nullptr);
	vkDestroyBuffer(Device::GetDevice(), stagingBuffer, nullptr);
}

void Renderer::CreateDescriptorSetLayout()
{
	// uniform buffer
	VkDescriptorSetLayoutBinding uboLayout{};
	uboLayout.binding = 0; // binding in the shader
	uboLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayout.descriptorCount = 1;
	uboLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayout.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 1> bindings{ uboLayout };

	VkDescriptorSetLayoutCreateInfo descriptorSetInfo{};
	descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	descriptorSetInfo.pBindings = bindings.data();

	THROW(vkCreateDescriptorSetLayout(Device::GetDevice(), &descriptorSetInfo, nullptr, &m_DescriptorSetLayout)
			  != VK_SUCCESS,
		"Failed to create descriptor set layout!");
}

void Renderer::CreateUniformBuffers()
{
	VkDeviceSize size = static_cast<uint64_t>(sizeof(UniformBufferObject));

	m_UniformBuffers.resize(m_Config.maxFramesInFlight);
	m_UniformBufferMemory.resize(m_Config.maxFramesInFlight);
	m_UniformBufferMapped.resize(m_Config.maxFramesInFlight);

	for (uint32_t i = 0; i < m_Config.maxFramesInFlight; ++i)
	{
		utils::CreateBuffer(size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_UniformBuffers[i],
			m_UniformBufferMemory[i]);

		vkMapMemory(Device::GetDevice(), m_UniformBufferMemory[i], 0, size, 0, &m_UniformBufferMapped[i]);
	}
}

void Renderer::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	// uniform buffer
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_Config.maxFramesInFlight);

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.maxSets = static_cast<uint32_t>(m_Config.maxFramesInFlight);
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();

	THROW(vkCreateDescriptorPool(Device::GetDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS,
		"Failed to create descriptor pool!");
}

void Renderer::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts{ m_Config.maxFramesInFlight, m_DescriptorSetLayout };

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(m_Config.maxFramesInFlight);
	descriptorSetAllocateInfo.pSetLayouts = layouts.data();

	// we create one descriptor set for each frame with the same layout
	m_DescriptorSets.resize(m_Config.maxFramesInFlight);
	THROW(vkAllocateDescriptorSets(Device::GetDevice(), &descriptorSetAllocateInfo, m_DescriptorSets.data())
			  != VK_SUCCESS,
		"Failed to allocate descriptor sets!")

	// configure descriptors in the descriptor sets
	for (size_t i = 0; i < m_Config.maxFramesInFlight; ++i)
	{
		// uniform buffer
		VkDescriptorBufferInfo descriptorBufferInfo{};
		descriptorBufferInfo.buffer = m_UniformBuffers[i];
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = sizeof(UniformBufferObject);

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
		// uniform buffer
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_DescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &descriptorBufferInfo;

		// updates the configurations of the descriptor sets
		vkUpdateDescriptorSets(
			Device::GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Renderer::UpdateUniformBuffer(uint32_t currentFrameIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f),
		static_cast<float>(m_SwapchainExtent.width) / static_cast<float>(m_SwapchainExtent.height),
		0.1f,
		10.0f);
	ubo.proj[1][1] *= -1;

	memcpy(m_UniformBufferMapped[currentFrameIndex], &ubo, sizeof(ubo));
}