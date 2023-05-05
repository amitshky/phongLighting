#include "renderer/renderer.h"


Renderer::Renderer(const char* title,
	const VulkanConfig& config,
	const std::shared_ptr<Window>& window)
	: m_VulkanContext{ std::make_unique<VulkanContext>(title, config, window) },
	  m_Device{ std::make_unique<Device>(config,
		  m_VulkanContext->GetInstance(),
		  m_VulkanContext->GetWindowSurface()) }
{}
