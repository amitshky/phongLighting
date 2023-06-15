#include "renderer/swapchain.h"

#include <algorithm>
#include "core/core.h"
#include "renderer/vulkanContext.h"
#include "renderer/device.h"
#include "utils/utils.h"


Swapchain::Swapchain(const std::shared_ptr<Window>& window)
	: m_Window{ window }
{
	Init();
}

Swapchain::~Swapchain()
{
	Cleanup();
	// Cleanup() is also called when recreating swapchain
	// but we don't recreate render pass
	vkDestroyRenderPass(Device::GetDevice(), m_RenderPass, nullptr);
}

void Swapchain::Init()
{
	CreateSwapchain();
	CreateSwapchainImageViews();

	CreateRenderPass();
	CreateColorResource();
	CreateDepthResource();
	CreateFramebuffers();
}

void Swapchain::Cleanup()
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

void Swapchain::RecreateSwapchain()
{
	while (m_Window->IsMinimized())
	{
		m_Window->WaitEvents();
	}

	Device::WaitIdle();
	Cleanup();

	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateColorResource();
	CreateDepthResource();
	CreateFramebuffers();
}

VkResult Swapchain::AcquireNextImageIndex(VkSemaphore imageAvailableSemaphore, uint32_t* pImageIndex)
{
	// acquire image from the swapchain
	// signals the semaphore
	return vkAcquireNextImageKHR(
		Device::GetDevice(), m_Swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, pImageIndex);
}

void Swapchain::Present(const VkSemaphore* pWaitSemaphores, uint32_t waitSemaphoreCount, const uint32_t* pImageIndices)
{
	std::array<VkSwapchainKHR, 1> swapchains{ m_Swapchain };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = waitSemaphoreCount;
	presentInfo.pWaitSemaphores = pWaitSemaphores;
	presentInfo.swapchainCount = static_cast<uint32_t>(swapchains.size());
	presentInfo.pSwapchains = swapchains.data();
	presentInfo.pImageIndices = pImageIndices;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(Device::GetPresentQueue(), &presentInfo);
}

void Swapchain::BeginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	auto clearValues = ClearAttachmentValues();
	SetViewport(commandBuffer);
	SetScissor(commandBuffer);

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.framebuffer = m_SwapchainFramebuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_SwapchainExtent;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Swapchain::EndRenderPass(VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);
}

std::array<VkClearValue, 3> Swapchain::ClearAttachmentValues()
{
	// clear values for each attachment
	std::array<VkClearValue, 3> clearValues;
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	clearValues[2].color = clearValues[0].color;

	return clearValues;
}

void Swapchain::SetScissor(VkCommandBuffer commandBuffer)
{
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapchainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Swapchain::SetViewport(VkCommandBuffer commandBuffer)
{
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapchainExtent.width);
	viewport.height = static_cast<float>(m_SwapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void Swapchain::CreateSwapchain()
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

void Swapchain::CreateSwapchainImageViews()
{
	m_SwapchainImageViews.resize(m_SwapchainImages.size());

	for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
	{
		m_SwapchainImageViews[i] =
			utils::CreateImageView(m_SwapchainImages[i], m_SwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void Swapchain::CreateRenderPass()
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

void Swapchain::CreateColorResource()
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

void Swapchain::CreateDepthResource()
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

void Swapchain::CreateFramebuffers()
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

VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}

	return availableFormats[0];
}

VkPresentModeKHR Swapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

VkFormat Swapchain::FindDepthFormat()
{
	return Device::FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
