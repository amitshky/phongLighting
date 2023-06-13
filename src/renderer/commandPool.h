#pragma once

#include <vulkan/vulkan.h>


class CommandPool
{
public:
	CommandPool(const CommandPool&) = delete;
	CommandPool& operator=(const CommandPool&) = delete;
	~CommandPool();

	static CommandPool* Create();
	static inline VkCommandPool Get() { return s_Instance->m_CommandPool; }

private:
	CommandPool();
	void CreateCommandPool();

private:
	static CommandPool* s_Instance;
	VkCommandPool m_CommandPool;
};