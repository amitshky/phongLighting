#include "renderer/renderer.h"

#include <array>
#include <numeric>
#include <algorithm>
#include "core/core.h"
#include "utils/utils.h"


Renderer::Renderer(const char* title, const VulkanConfig& config, const std::shared_ptr<Window>& window)
	: m_Window{ window }
{
	Init(title, config);
}

Renderer::~Renderer()
{
	CleanupSwapchain();

	delete m_Device;
	delete m_VulkanContext;
}

void Renderer::Init(const char* title, const VulkanConfig& config)
{
	m_VulkanContext = VulkanContext::Create(title, config, m_Window);
	m_Device = Device::Create(config, m_Window->GetWindowSurface());

	CreateSwapchain();
	CreateSwapchainImageViews();
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

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = VulkanContext::GetWindowSurface();
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value())
	{
		uint32_t indicesArr[]{ queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = indicesArr;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	THROW(vkCreateSwapchainKHR(Device::GetDevice(), &swapchainCreateInfo, nullptr, &m_Swapchain) != VK_SUCCESS,
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
	for (const auto& imageView : m_SwapchainImageViews)
		vkDestroyImageView(Device::GetDevice(), imageView, nullptr);

	// swapchain images are destroyed with `vkDestroySwapchainKHR()`
	vkDestroySwapchainKHR(Device::GetDevice(), m_Swapchain, nullptr);
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