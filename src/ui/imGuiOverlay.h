#pragma once

#include <vulkan/vulkan.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

class ImGuiOverlay
{
public:
	static void Init(uint32_t imageCount, VkRenderPass renderPass);
	static void Cleanup();
	static void Begin();
	static void End(VkCommandBuffer commandBuffer);

private:
	static void CheckVkResult(VkResult err);
};