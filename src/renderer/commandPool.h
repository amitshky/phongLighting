#pragma once

#include <vulkan/vulkan.h>
#include <memory>


class CommandPool
{
public:
	CommandPool();
	CommandPool(const CommandPool&) = delete;
	CommandPool& operator=(const CommandPool&) = delete;
	~CommandPool();

	static std::shared_ptr<CommandPool> Create();
	static inline VkCommandPool Get() { return s_Instance->m_CommandPool; }

private:
	void CreateCommandPool();

private:
	static std::shared_ptr<CommandPool> s_Instance;
	VkCommandPool m_CommandPool;
};