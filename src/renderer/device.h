#pragma once

#include <optional>
#include <memory>
#include "renderer/vulkanContext.h"


struct QueueFamilyIndices
{
	// std::optional contains no value until you assign something to it
	// we can check if it contains a value by calling has_value()
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	[[nodiscard]] inline bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
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
	Device(const VulkanConfig& config, VkSurfaceKHR windowSurface);
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
	~Device();

	static std::shared_ptr<Device> Create(const VulkanConfig& config, VkSurfaceKHR windowSurface);

	static inline VkDevice GetDevice() { return s_Instance->m_DeviceVk; }
	static inline VkPhysicalDevice GetPhysicalDevice() { return s_Instance->m_PhysicalDevice; }

	static inline VkPhysicalDeviceProperties GetDeviceProperties() { return s_Instance->m_PhysicalDeviceProperties; }

	static inline VkQueue GetGraphicsQueue() { return s_Instance->m_GraphicsQueue; }
	static inline VkQueue GetPresentQueue() { return s_Instance->m_PresentQueue; }
	static inline QueueFamilyIndices GetQueueFamilyIndices() { return s_Instance->m_QueueFamilyIndices; }
	static inline VkSampleCountFlagBits GetMSAASamplesCount() { return s_Instance->m_MsaaSamples; }

	// waits for the device to finish operations
	static inline void WaitIdle() { vkDeviceWaitIdle(s_Instance->m_DeviceVk); }

	// device details functions
	static uint32_t
		FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface);
	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface);
	static VkFormat FindSupportedFormat(const std::vector<VkFormat>& canditateFormats,
		VkImageTiling tiling,
		VkFormatFeatureFlags features);

private:
	void PickPhysicalDevice();
	void CreateLogicalDevice();

	bool IsDeviceSuitable(VkPhysicalDevice physicalDevice);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

	VkSampleCountFlagBits GetMaxUsableSampleCount();

private:
	const VulkanConfig m_Config;
	VkInstance m_VulkanInstance;
	VkSurfaceKHR m_WindowSurface;

	static std::shared_ptr<Device> s_Instance;

	QueueFamilyIndices m_QueueFamilyIndices;

	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_DeviceVk;

	VkPhysicalDeviceProperties m_PhysicalDeviceProperties;

	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	VkSampleCountFlagBits m_MsaaSamples;
};
