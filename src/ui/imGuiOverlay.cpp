#include "ui/imGuiOverlay.h"

#include "core/core.h"
#include "core/application.h"
#include "renderer/vulkanContext.h"
#include "renderer/device.h"
#include "renderer/descriptor.h"


void ImGuiOverlay::Init(uint32_t imageCount, VkRenderPass renderPass)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_DockingEnabled; // needs docking branch of ImGui repo

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(Application::GetInstance().GetWindow().GetWindowHandle(), true);
	ImGui_ImplVulkan_InitInfo info{};
	info.Instance = VulkanContext::GetInstance();
	info.PhysicalDevice = Device::GetPhysicalDevice();
	info.Device = Device::GetDevice();
	info.QueueFamily = Device::GetQueueFamilyIndices().graphicsFamily.value();
	info.Queue = Device::GetGraphicsQueue();
	info.DescriptorPool = DescriptorSet::GetDescriptorPool();
	info.PipelineCache = VK_NULL_HANDLE;
	info.Subpass = 0;
	info.MinImageCount = imageCount;
	info.ImageCount = imageCount;
	info.MSAASamples = Device::GetMSAASamplesCount();
	info.Allocator = nullptr;
	info.CheckVkResultFn = CheckVkResult;
	ImGui_ImplVulkan_Init(&info, renderPass);

	VkCommandBuffer cmdBuff = CommandBuffer::BeginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(cmdBuff);
	CommandBuffer::EndSingleTimeCommands(cmdBuff);

	Device::WaitIdle();
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiOverlay::Cleanup()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiOverlay::Begin()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiOverlay::End(VkCommandBuffer commandBuffer)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiOverlay::CheckVkResult(VkResult err)
{
	if (err == VK_SUCCESS)
		return;

	Logger::Error("ImGui::Vulkan: {}\n", static_cast<uint32_t>(err));
}