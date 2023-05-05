#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include "renderer/vulkanContext.h"
#include "renderer/device.h"


class Renderer
{
public:
	explicit Renderer(const char* title,
		const VulkanConfig& config,
		const std::shared_ptr<Window>& window);

private:
	std::unique_ptr<VulkanContext> m_VulkanContext;
	std::unique_ptr<Device> m_Device;
};