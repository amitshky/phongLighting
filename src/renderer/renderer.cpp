#include "renderer/renderer.h"

#include <array>
#include <numeric>
#include <chrono>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "stb_image/stb_image.h"
#include "core/core.h"
#include "renderer/shader.h"
#include "utils/utils.h"
#include "ui/imGuiOverlay.h"

constexpr uint64_t NUM_CUBES = 3;

const std::vector<Vertex> vertexData{
  // front
	{{ -0.5f, 0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f }}, // 3
	{ { 0.5f, 0.5f, 0.5f },    { 0.0f, 0.0f, 1.0f },  { 1.0f, 0.0f }}, // 2
	{ { 0.5f, -0.5f, 0.5f },   { 0.0f, 0.0f, 1.0f },  { 1.0f, 1.0f }}, // 1
	{ { 0.5f, -0.5f, 0.5f },   { 0.0f, 0.0f, 1.0f },  { 1.0f, 1.0f }}, // 1
	{ { -0.5f, -0.5f, 0.5f },  { 0.0f, 0.0f, 1.0f },  { 0.0f, 1.0f }}, // 0
	{ { -0.5f, 0.5f, 0.5f },   { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f }}, // 3

  // right
	{ { 0.5f, 0.5f, 0.5f },    { 1.0f, 0.0f, 0.0f },  { 0.0f, 0.0f }}, // 2
	{ { 0.5f, 0.5f, -0.5f },   { 1.0f, 0.0f, 0.0f },  { 1.0f, 0.0f }}, // 6
	{ { 0.5f, -0.5f, -0.5f },  { 1.0f, 0.0f, 0.0f },  { 1.0f, 1.0f }}, // 5
	{ { 0.5f, -0.5f, -0.5f },  { 1.0f, 0.0f, 0.0f },  { 1.0f, 1.0f }}, // 5
	{ { 0.5f, -0.5f, 0.5f },   { 1.0f, 0.0f, 0.0f },  { 0.0f, 1.0f }}, // 1
	{ { 0.5f, 0.5f, 0.5f },    { 1.0f, 0.0f, 0.0f },  { 0.0f, 0.0f }}, // 2

  // back
	{ { 0.5f, 0.5f, -0.5f },   { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }}, // 6
	{ { -0.5f, 0.5f, -0.5f },  { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }}, // 7
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }}, // 4
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }}, // 4
	{ { 0.5f, -0.5f, -0.5f },  { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }}, // 5
	{ { 0.5f, 0.5f, -0.5f },   { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }}, // 6

  // left
	{ { -0.5f, 0.5f, -0.5f },  { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}, // 7
	{ { -0.5f, 0.5f, 0.5f },   { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }}, // 3
	{ { -0.5f, -0.5f, 0.5f },  { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }}, // 0
	{ { -0.5f, -0.5f, 0.5f },  { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }}, // 0
	{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }}, // 4
	{ { -0.5f, 0.5f, -0.5f },  { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}, // 7

  // top
	{ { -0.5f, 0.5f, -0.5f },  { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f }}, // 7
	{ { 0.5f, 0.5f, -0.5f },   { 0.0f, 1.0f, 0.0f },  { 1.0f, 0.0f }}, // 6
	{ { 0.5f, 0.5f, 0.5f },    { 0.0f, 1.0f, 0.0f },  { 1.0f, 1.0f }}, // 2
	{ { 0.5f, 0.5f, 0.5f },    { 0.0f, 1.0f, 0.0f },  { 1.0f, 1.0f }}, // 2
	{ { -0.5f, 0.5f, 0.5f },   { 0.0f, 1.0f, 0.0f },  { 0.0f, 1.0f }}, // 3
	{ { -0.5f, 0.5f, -0.5f },  { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f }}, // 7

  // bottom
	{ { -0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }}, // 0
	{ { 0.5f, -0.5f, 0.5f },   { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }}, // 1
	{ { 0.5f, -0.5f, -0.5f },  { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}, // 5
	{ { 0.5f, -0.5f, -0.5f },  { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}, // 5
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }}, // 4
	{ { -0.5f, -0.5f, 0.5f },  { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }}, // 0
};

const auto [indices, vertices] = utils::GetModelData(vertexData);

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

	m_Swapchain = std::make_unique<Swapchain>(m_Window);

	m_VertexBuffer = std::make_unique<VertexBuffer>(vertices);
	m_IndexBuffer = std::make_unique<IndexBuffer>(indices);

	VkDeviceSize uboSize = static_cast<uint64_t>(sizeof(UniformBufferObject));
	VkDeviceSize minAlignment = Device::GetDeviceProperties().limits.minUniformBufferOffsetAlignment;
	m_DUbo.Init(minAlignment, NUM_CUBES);

	m_UniformBuffers.reserve(m_Config.maxFramesInFlight);
	m_DynamicUniformBuffers.reserve(m_Config.maxFramesInFlight);
	for (uint64_t i = 0; i < m_Config.maxFramesInFlight; ++i)
	{
		m_UniformBuffers.emplace_back(
			uboSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uboSize);
		m_DynamicUniformBuffers.emplace_back(m_DUbo.GetBufferSize(),
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_DUbo.GetAlignment());
	}

	m_Texture = std::make_unique<Texture2D>("assets/textures/checkerboard.png");

	DescriptorSet::CreateDescriptorPool();
	m_DescriptorSet = std::make_unique<DescriptorSet>(m_Config.maxFramesInFlight);
	m_DescriptorSet->SetupLayout({
		DescriptorSet::CreateLayout(DescriptorType::UNIFORM_BUFFER, //
			ShaderType::VERTEX,
			0,
			1,
			m_UniformBuffers.data(),
			static_cast<uint32_t>(m_UniformBuffers.size()),
			nullptr,
			0), //
		DescriptorSet::CreateLayout(DescriptorType::UNIFORM_BUFFER_DYNAMIC, //
			ShaderType::VERTEX,
			1,
			1,
			m_DynamicUniformBuffers.data(),
			static_cast<uint32_t>(m_DynamicUniformBuffers.size()),
			nullptr,
			0), //
		DescriptorSet::CreateLayout(DescriptorType::COMBINED_IMAGE_SAMPLER, //
			ShaderType::FRAGMENT,
			2,
			1,
			nullptr,
			0,
			m_Texture.get(),
			1) //
	});
	m_DescriptorSet->Create();

	m_Pipeline = std::make_unique<Pipeline>("assets/shaders/cube.vert.spv",
		"assets/shaders/cube.frag.spv",
		m_DescriptorSet->GetPipelineLayout(),
		m_Swapchain->GetRenderPass());

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
}

void Renderer::Draw(float deltatime)
{
	BeginScene();

	m_Pipeline->Bind(m_ActiveCommandBuffer);
	m_VertexBuffer->Bind(m_ActiveCommandBuffer);
	m_IndexBuffer->Bind(m_ActiveCommandBuffer);

	for (uint64_t i = 0; i < NUM_CUBES; ++i)
	{
		uint32_t dynamicOffset = i * m_DUbo.GetAlignment();
		m_DescriptorSet->Bind(m_ActiveCommandBuffer, m_CurrentFrameIndex, 1, &dynamicOffset);
		m_IndexBuffer->Draw(m_ActiveCommandBuffer);
	}

	UpdateUniformBuffers(m_CurrentFrameIndex);

	ImGuiOverlay::Begin();

	ImGui::ShowDemoWindow();
	ImGui::Begin("hello");
	ImGui::End();

	ImGuiOverlay::End(m_ActiveCommandBuffer);

	EndScene();

	m_Camera->OnUpdate(deltatime);
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

void Renderer::UpdateUniformBuffers(uint32_t currentFrameIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	m_Ubo.lightPos = glm::vec3(0.0f, 0.0f, 2.0f);
	m_Ubo.viewPos = m_Camera->GetCameraPosition();
	m_Ubo.viewProjMat = m_Camera->GetViewProjectionMatrix();
	m_UniformBuffers[currentFrameIndex].Map(&m_Ubo);

	for (uint64_t i = 0; i < NUM_CUBES; ++i)
	{
		// get a pointer to the aligned offset
		glm::mat4* modelMatPtr = m_DUbo.GetModelMatPtr(i);
		*modelMatPtr = glm::translate(glm::mat4(1.0f), glm::vec3(i * 2 - 2.0f, 0.0f, 0.0f))
					   * glm::rotate(glm::mat4(1.0f), time * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4* normMatPtr = m_DUbo.GetNormalMatPtr(i);
		*normMatPtr = glm::inverseTranspose(*modelMatPtr); // 4x4 converted to 3x3 in the vertex shader
	}

	m_DynamicUniformBuffers[currentFrameIndex].Map(m_DUbo.buffer);
}