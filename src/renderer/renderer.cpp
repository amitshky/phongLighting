#include "renderer/renderer.h"

#include <cmath>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "core/core.h"
#include "utils/utils.h"
#include "ui/imGuiOverlay.h"


constexpr uint64_t NUM_INSTANCES = 3;

Renderer::Renderer(const char* title, const VulkanConfig& config, const std::shared_ptr<Window>& window)
	: m_Config{ config },
	  m_Window{ window }
{
	Init(title);
}

Renderer::~Renderer()
{
	Cleanup();
}

void Renderer::Init(const char* title)
{
	m_VulkanContext = VulkanContext::Create(title, m_Config, m_Window);
	m_Device = Device::Create(m_Config, m_Window->GetWindowSurface());
	m_CommandPool = CommandPool::Create();
	DescriptorPool::Init();

	m_Swapchain = std::make_unique<Swapchain>(m_Window);

	m_BackpackModel = std::make_unique<Model>("assets/models/backpack/backpack.obj",
		m_Swapchain->GetRenderPass(),
		m_Config.maxFramesInFlight,
		NUM_INSTANCES,
		false);
	m_CerberusModel = std::make_unique<Model>("assets/models/Cerberus/Cerberus_LP.FBX",
		m_Swapchain->GetRenderPass(),
		m_Config.maxFramesInFlight,
		NUM_INSTANCES,
		true);

	m_Cube = std::make_unique<Cube>(m_Swapchain->GetRenderPass(), m_Config.maxFramesInFlight, NUM_INSTANCES);
	m_LightCube = std::make_unique<LightCube>(m_Swapchain->GetRenderPass(), m_Config.maxFramesInFlight, NUM_INSTANCES);

	VkDeviceSize uboSize = sizeof(UniformBufferObject);
	VkDeviceSize minAlignment = Device::GetDeviceProperties().limits.minUniformBufferOffsetAlignment;
	m_DUbo.Init(minAlignment, NUM_INSTANCES);

	m_CommandBuffer = std::make_unique<CommandBuffer>(m_Config.maxFramesInFlight);

	CreateSyncObjects();

	m_Camera = std::make_unique<Camera>(
		static_cast<float>(m_Swapchain->GetWidth()) / static_cast<float>(m_Swapchain->GetHeight()));

	ImGuiOverlay::Init(m_Config.maxFramesInFlight, m_Swapchain->GetRenderPass());
}

void Renderer::Cleanup()
{
	Device::WaitIdle();

	ImGuiOverlay::Cleanup();

	for (size_t i = 0; i < m_Config.maxFramesInFlight; ++i)
	{
		vkDestroySemaphore(Device::GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(Device::GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(Device::GetDevice(), m_InFlightFences[i], nullptr);
	}

	DescriptorPool::Cleanup();
}

void Renderer::Draw(float deltatime, uint32_t fpsCount)
{
	BeginScene();

	uint32_t dynamicOffset = 0 * m_DUbo.GetAlignment();
	m_BackpackModel->Draw(m_ActiveCommandBuffer, m_CurrentFrameIndex, 1, &dynamicOffset);

	dynamicOffset = 1 * m_DUbo.GetAlignment();
	m_CerberusModel->Draw(m_ActiveCommandBuffer, m_CurrentFrameIndex, 1, &dynamicOffset);

	dynamicOffset = 2 * m_DUbo.GetAlignment();
	m_Cube->Draw(m_ActiveCommandBuffer, m_CurrentFrameIndex, 1, &dynamicOffset);

	m_LightCube->Draw(m_ActiveCommandBuffer, m_CurrentFrameIndex);

	UpdateUniformBuffers(m_CurrentFrameIndex);

	OnUIRender(fpsCount);
	EndScene();

	m_Camera->OnUpdate(deltatime);
}

void Renderer::UpdateUniformBuffers(uint32_t currentFrameIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	glm::vec3 lightPos{ 1.5 * std::sinf(time), 0.0f, 1.5 * std::cosf(time) };

	m_Ubo.lightPos = lightPos;
	m_Ubo.viewPos = m_Camera->GetCameraPosition();
	m_Ubo.viewProjMat = m_Camera->GetViewProjectionMatrix();

	// backpack model
	uint32_t i = 0;
	// get a pointer to the aligned offset
	glm::mat4* modelMatPtr = m_DUbo.GetModelMatPtr(i);
	*modelMatPtr = glm::translate(glm::mat4(1.0f), m_BackpackPos);
	*modelMatPtr = glm::rotate(*modelMatPtr, glm::radians(m_BackpackRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
	*modelMatPtr = glm::rotate(*modelMatPtr, glm::radians(m_BackpackRotateY), glm::vec3(0.0f, 1.0f, 0.0f));
	*modelMatPtr = glm::rotate(*modelMatPtr, glm::radians(m_BackpackRotateZ), glm::vec3(0.0f, 0.0f, 1.0f));
	*modelMatPtr = glm::scale(*modelMatPtr, glm::vec3(0.2f));
	glm::mat4* normMatPtr = m_DUbo.GetNormalMatPtr(i);
	*normMatPtr = glm::inverseTranspose(*modelMatPtr); // 4x4 converted to 3x3 in the vertex shader

	// cerberus model
	i = 1;
	modelMatPtr = m_DUbo.GetModelMatPtr(i);
	*modelMatPtr = glm::translate(glm::mat4(1.0f), m_CerberusPos);
	*modelMatPtr = glm::rotate(*modelMatPtr, glm::radians(m_CerberusRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
	*modelMatPtr = glm::rotate(*modelMatPtr, glm::radians(m_CerberusRotateY), glm::vec3(0.0f, 1.0f, 0.0f));
	*modelMatPtr = glm::rotate(*modelMatPtr, glm::radians(m_CerberusRotateZ), glm::vec3(0.0f, 0.0f, 1.0f));
	*modelMatPtr = glm::scale(*modelMatPtr, glm::vec3(0.005f));
	normMatPtr = m_DUbo.GetNormalMatPtr(i);
	*normMatPtr = glm::inverseTranspose(*modelMatPtr); // 4x4 converted to 3x3 in the vertex shader

	// cube
	i = 2;
	modelMatPtr = m_DUbo.GetModelMatPtr(i);
	*modelMatPtr = glm::translate(glm::mat4(1.0f), m_CubePos);
	*modelMatPtr = glm::rotate(*modelMatPtr, glm::radians(m_CubeRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
	*modelMatPtr = glm::rotate(*modelMatPtr, glm::radians(m_CubeRotateY), glm::vec3(0.0f, 1.0f, 0.0f));
	*modelMatPtr = glm::rotate(*modelMatPtr, glm::radians(m_CubeRotateZ), glm::vec3(0.0f, 0.0f, 1.0f));
	*modelMatPtr = glm::scale(*modelMatPtr, glm::vec3(0.5f));
	normMatPtr = m_DUbo.GetNormalMatPtr(i);
	*normMatPtr = glm::inverseTranspose(*modelMatPtr); // 4x4 converted to 3x3 in the vertex shader

	m_BackpackModel->UpdateUniformBuffers(m_Ubo, m_DUbo, currentFrameIndex);
	m_CerberusModel->UpdateUniformBuffers(m_Ubo, m_DUbo, currentFrameIndex);
	m_Cube->UpdateUniformBuffers(m_Ubo, m_DUbo, currentFrameIndex);

	// light cube
	m_LightCubeUbo.transformationMat = m_Camera->GetViewProjectionMatrix();
	m_LightCubeUbo.transformationMat = glm::translate(m_LightCubeUbo.transformationMat, lightPos);
	m_LightCubeUbo.transformationMat = glm::scale(m_LightCubeUbo.transformationMat, glm::vec3(0.1f));
	m_LightCube->UpdateUniformBuffers(m_LightCubeUbo, currentFrameIndex);
}

void Renderer::OnUIRender(uint32_t fpsCount)
{
	ImGuiOverlay::Begin();

	ImGui::Begin("Profiler");
	ImGui::Text("%.2f ms/frame (%d fps)", (1000.0f / fpsCount), fpsCount);
	ImGui::End();

	ImGui::Begin("Properties");

	ImGui::SeparatorText("Backpack:");
	ImGui::Text("Position:");
	ImGui::SameLine();
	// `##` in the label is for id of the label
	// `Name###ID`
	ImGui::SliderFloat3("##backpack_position", glm::value_ptr(m_BackpackPos), -5.0f, 5.0f);
	ImGui::Text("Rotate:");
	ImGui::Text("X-axis:");
	ImGui::SameLine();
	ImGui::SliderFloat("##backpack_x_axis", &m_BackpackRotateX, -180.0f, 180.0f);
	ImGui::Text("Y-axis:");
	ImGui::SameLine();
	ImGui::SliderFloat("##backpack_y_axis", &m_BackpackRotateY, -180.0f, 180.0f);
	ImGui::Text("Z-axis:");
	ImGui::SameLine();
	ImGui::SliderFloat("##backpack_z_axis", &m_BackpackRotateZ, -180.0f, 180.0f);
	ImGui::Separator();

	ImGui::SeparatorText("Cerberus:");
	ImGui::Text("Position");
	ImGui::SameLine();
	ImGui::SliderFloat3("##cerberus_position", glm::value_ptr(m_CerberusPos), -5.0f, 5.0f);
	ImGui::Text("Rotate:");
	ImGui::Text("X-axis:");
	ImGui::SameLine();
	ImGui::SliderFloat("##cerberus_x_axis", &m_CerberusRotateX, -180.0f, 180.0f);
	ImGui::Text("Y-axis:");
	ImGui::SameLine();
	ImGui::SliderFloat("##cerberus_y_axis", &m_CerberusRotateY, -180.0f, 180.0f);
	ImGui::Text("Z-axis:");
	ImGui::SameLine();
	ImGui::SliderFloat("##cerberus_z_axis", &m_CerberusRotateZ, -180.0f, 180.0f);
	ImGui::Separator();

	ImGui::SeparatorText("Cube:");
	ImGui::Text("Position");
	ImGui::SameLine();
	ImGui::SliderFloat3("##cube_position", glm::value_ptr(m_CubePos), -5.0f, 5.0f);
	ImGui::Text("Rotate:");
	ImGui::Text("X-axis:");
	ImGui::SameLine();
	ImGui::SliderFloat("##cube_x_axis", &m_CubeRotateX, -180.0f, 180.0f);
	ImGui::Text("Y-axis:");
	ImGui::SameLine();
	ImGui::SliderFloat("##cube_y_axis", &m_CubeRotateY, -180.0f, 180.0f);
	ImGui::Text("Z-axis:");
	ImGui::SameLine();
	ImGui::SliderFloat("##cube_z_axis", &m_CubeRotateZ, -180.0f, 180.0f);
	ImGui::Separator();

	ImGui::End();

	ImGuiOverlay::End(m_ActiveCommandBuffer);
}

void Renderer::OnResize(int /*unused*/, int /*unused*/)
{
	m_Swapchain->RecreateSwapchain();
	m_Camera->SetAspectRatio(
		static_cast<float>(m_Swapchain->GetWidth()) / static_cast<float>(m_Swapchain->GetHeight()));
}

void Renderer::OnMouseMove(double xpos, double ypos)
{
	m_Camera->OnMouseMove(xpos, ypos);
}

void Renderer::BeginScene()
{
	// wait for previous frame to signal the fence
	vkWaitForFences(Device::GetDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);

	VkResult result =
		m_Swapchain->AcquireNextImageIndex(m_ImageAvailableSemaphores[m_CurrentFrameIndex], // signal this semaphore
			&m_NextFrameIndex);
	THROW(result != VK_SUCCESS, "Failed to acquire swapchain image!")

	// resetting the fence has been set after the result has been checked to
	// avoid a deadlock reset the fence to unsignaled state
	vkResetFences(Device::GetDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex]);

	m_ActiveCommandBuffer = m_CommandBuffer->GetBufferAt(m_CurrentFrameIndex);
	m_CommandBuffer->Begin(m_CurrentFrameIndex);
	m_Swapchain->BeginRenderPass(m_ActiveCommandBuffer, m_NextFrameIndex);
}

void Renderer::EndScene()
{
	m_Swapchain->EndRenderPass(m_ActiveCommandBuffer);
	m_CommandBuffer->End(m_CurrentFrameIndex);

	std::array<VkPipelineStageFlags, 1> waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	m_CommandBuffer->Submit(m_InFlightFences[m_CurrentFrameIndex],
		&m_ImageAvailableSemaphores[m_CurrentFrameIndex], // wait on this semaphore
		1,
		&m_RenderFinishedSemaphores[m_CurrentFrameIndex], // signal this semaphore
		1,
		waitStages.data(),
		m_CurrentFrameIndex);

	m_Swapchain->Present(&m_RenderFinishedSemaphores[m_CurrentFrameIndex], // wait on this semaphore
		1,
		&m_NextFrameIndex);

	// update current frame index
	m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_Config.maxFramesInFlight;
}

void Renderer::CreateSyncObjects()
{
	m_ImageAvailableSemaphores.resize(m_Config.maxFramesInFlight);
	m_RenderFinishedSemaphores.resize(m_Config.maxFramesInFlight);
	m_InFlightFences.resize(m_Config.maxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// create the fence in signaled state so that the first frame doesnt have to wait
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < m_Config.maxFramesInFlight; ++i)
	{
		THROW(vkCreateSemaphore(Device::GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i])
					  != VK_SUCCESS
				  || vkCreateSemaphore(Device::GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i])
						 != VK_SUCCESS
				  || vkCreateFence(Device::GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS,
			"Failed to create synchronization objects!")
	}
}
