#include "renderer/commandPool.h"

#include "core/core.h"
#include "renderer/vulkanContext.h"
#include "renderer/device.h"


CommandPool* CommandPool::s_Instance = nullptr;

CommandPool::CommandPool()
{
	s_Instance = this;
	CreateCommandPool();
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(Device::GetDevice(), m_CommandPool, nullptr);
}

CommandPool* CommandPool::Create()
{
	if (s_Instance == nullptr)
		return new CommandPool{};

	return s_Instance;
}

void CommandPool::CreateCommandPool()
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