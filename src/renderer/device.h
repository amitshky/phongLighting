#pragma once

#include <optional>
#include "renderer/vulkanContext.h"


struct QueueFamilyIndices
{
	// std::optional contains no value until you assign something to it
	// we can check if it contains a value by calling has_value()
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	inline bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Device
{
public:
	~Device();

	static Device* Create(const VulkanConfig& config, VkSurfaceKHR windowSurface);

	static inline VkDevice GetDevice() { return s_Device->m_DeviceVk; }
	static inline VkPhysicalDevice GetPhysicalDevice() { return s_Device->m_PhysicalDevice; }

	static inline VkQueue GetGraphicsQueue() { return s_Device->m_GraphicsQueue; }
	static inline VkQueue GetPresentQueue() { return s_Device->m_GraphicsQueue; }
	static inline VkSampleCountFlagBits GetMSAASamplesCount() { return s_Device->m_MsaaSamples; }

	// device details functions
	static uint32_t
		FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface);
	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface);

private:
	Device(const VulkanConfig& config, VkSurfaceKHR windowSurface);

	void PickPhysicalDevice();
	void CreateLogicalDevice();

	bool IsDeviceSuitable(VkPhysicalDevice physicalDevice);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

	VkSampleCountFlagBits GetMaxUsableSampleCount();

private:
	const VulkanConfig m_Config;
	VkInstance m_VulkanInstance;
	VkSurfaceKHR m_WindowSurface;

	static Device* s_Device;

	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_DeviceVk;

	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	VkSampleCountFlagBits m_MsaaSamples;
};
