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

	CreateCommandBuffers();
	CreateSyncObjects();

	m_Camera = std::make_unique<Camera>(
		static_cast<float>(m_Swapchain->GetWidth()) / static_cast<float>(m_Swapchain->GetHeight()));
}

void Renderer::Cleanup()
{
	Device::WaitIdle();

	for (size_t i = 0; i < m_Config.maxFramesInFlight; ++i)
	{
		vkDestroySemaphore(Device::GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(Device::GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(Device::GetDevice(), m_InFlightFences[i], nullptr);
	}

	// destroying objects
	m_Pipeline.reset();
	m_DescriptorSet.reset();
	m_Texture.reset();
	m_UniformBuffers.clear();
	m_DynamicUniformBuffers.clear();
	m_VertexBuffer.reset();
	m_IndexBuffer.reset();
	m_Swapchain.reset();

	// TODO: make these shared_ptr
	delete m_CommandPool;
	delete m_Device;
	delete m_VulkanContext;
}

void Renderer::Draw(float deltatime)
{
	// wait for previous frame to signal the fence
	vkWaitForFences(Device::GetDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);

	uint32_t nextImageIndex = 0;
	VkResult result =
		m_Swapchain->AcquireNextImageIndex(m_ImageAvailableSemaphores[m_CurrentFrameIndex], &nextImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// we cannot simply return here, because the queue is never
		// submitted and thus the fences are never signaled , causing a
		// deadlock; to solve this we delay resetting the fences until
		// after we check the swapchain
		m_Swapchain->RecreateSwapchain();
		return;
	}
	THROW(result != VK_SUCCESS, "Failed to acquire swapchain image!")

	// resetting the fence has been set after the result has been checked to
	// avoid a deadlock reset the fence to unsignaled state
	vkResetFences(Device::GetDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex]);

	// record command buffer
	vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrameIndex], 0);
	RecordCommandBuffers(m_CommandBuffers[m_CurrentFrameIndex], nextImageIndex);

	UpdateUniformBuffers(m_CurrentFrameIndex);

	// submit the command buffer
	std::array<VkSemaphore, 1> waitSemaphores{ m_ImageAvailableSemaphores[m_CurrentFrameIndex] };
	std::array<VkSemaphore, 1> signalSemaphores{ m_RenderFinishedSemaphores[m_CurrentFrameIndex] };
	std::array<VkPipelineStageFlags, 1> waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrameIndex];
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	// signals the fence after executing the command buffer
	THROW(
		vkQueueSubmit(Device::GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrameIndex]) != VK_SUCCESS,
		"Failed to submit draw command buffer!")

	// present the image
	std::array<VkSwapchainKHR, 1> swapchains{ m_Swapchain->GetHandle() };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	presentInfo.pWaitSemaphores = signalSemaphores.data();
	presentInfo.swapchainCount = static_cast<uint32_t>(swapchains.size());
	presentInfo.pSwapchains = swapchains.data();
	presentInfo.pImageIndices = &nextImageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(Device::GetPresentQueue(), &presentInfo);

	m_Camera->OnUpdate(deltatime);
	// update current frame index
	m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_Config.maxFramesInFlight;
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

void Renderer::CreateCommandBuffers()
{
	m_CommandBuffers.resize(m_Config.maxFramesInFlight);

	VkCommandBufferAllocateInfo cmdBuffAllocInfo{};
	cmdBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBuffAllocInfo.commandPool = CommandPool::Get();
	cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBuffAllocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	THROW(vkAllocateCommandBuffers(Device::GetDevice(), &cmdBuffAllocInfo, m_CommandBuffers.data()) != VK_SUCCESS,
		"Failed to allocate command buffers!")
}

void Renderer::RecordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo cmdBuffBeginInfo{};
	cmdBuffBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	THROW(vkBeginCommandBuffer(commandBuffer, &cmdBuffBeginInfo) != VK_SUCCESS,
		"Failed to begin recording command buffer!")

	m_Swapchain->BeginRenderPass(commandBuffer, imageIndex);
	m_Pipeline->Bind(commandBuffer);

	m_VertexBuffer->Bind(commandBuffer);
	m_IndexBuffer->Bind(commandBuffer);

	for (uint64_t i = 0; i < NUM_CUBES; ++i)
	{
		uint32_t dynamicOffset = i * m_DUbo.GetAlignment();
		m_DescriptorSet->Bind(commandBuffer, m_CurrentFrameIndex, 1, &dynamicOffset);
		m_IndexBuffer->Draw(commandBuffer);
	}

	m_Swapchain->EndRenderPass(commandBuffer);
	THROW(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS, "Failed to record command buffer!");
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
		*normMatPtr = glm::inverseTranspose(*modelMatPtr); // 4x4, converted to 3x3 in the vertex shader
	}
	m_DynamicUniformBuffers[currentFrameIndex].Map(m_DUbo.buffer);
}
