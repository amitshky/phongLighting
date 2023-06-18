#include "core/application.h"

#include <chrono>
#include "core/core.h"
#include "core/input.h"
#include "ui/imGuiOverlay.h"


Application* Application::s_Instance = nullptr;

Application::Application(const char* title, uint32_t, uint32_t)
{
	Init(title);
}

void Application::Init(const char* title)
{
	s_Instance = this;
	Logger::Info("{} application initialized!", title);

	m_Window = std::make_shared<Window>(WindowProps{ title });
	// set window event callbacks
	// TODO: create only one function that dispatches all the events
	m_Window->SetCloseEventCallbackFn(BIND_EVENT_FN(Application::OnCloseEvent));
	m_Window->SetResizeEventCallbackFn(BIND_EVENT_FN(Application::OnResizeEvent));
	m_Window->SetMouseEventCallbackFn(BIND_EVENT_FN(Application::OnMouseMoveEvent));
	m_Window->SetMouseButtonCallbackFn(BIND_EVENT_FN(Application::OnMouseButtonEvent));
	m_Window->SetMouseScrollCallbackFn(BIND_EVENT_FN(Application::OnMouseScrollEvent));
	m_Window->SetKeyEventCallbackFn(BIND_EVENT_FN(Application::OnKeyEvent));

	const VulkanConfig config{
#ifdef NDEBUG // release mode
		false, // validation layers disabled
#else
		true, // validation layers enabled
#endif
		2, // max frames in flight
		{ "VK_LAYER_KHRONOS_validation" }, // validation layers
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME } // device extensions
	};

	m_Renderer = std::make_unique<Renderer>(title, config, m_Window);
}

Application* Application::Create(const char* title)
{
	if (Application::s_Instance == nullptr)
		return new Application{ title };

	return Application::s_Instance;
}

void Application::Run()
{
	m_LastFrameTime = std::chrono::high_resolution_clock::now();

	while (m_IsRunning)
	{
		float deltatime = CalcDeltaTime();

		m_Renderer->Draw(deltatime, m_LastFPS);
		m_Window->OnUpdate();
		ProcessInput();
	}
}

float Application::CalcDeltaTime()
{
	++m_FrameCounter;
	std::chrono::time_point<std::chrono::high_resolution_clock> currentFrameTime =
		std::chrono::high_resolution_clock::now();
	float deltatime =
		std::chrono::duration<float, std::chrono::milliseconds::period>(currentFrameTime - m_LastFrameTime).count();

	// calc fps every 1000ms
	if (deltatime > 1000.0f)
	{
		m_LastFPS = static_cast<uint32_t>(static_cast<float>(m_FrameCounter) * (1000.0f / deltatime));
		m_FrameCounter = 0;
		m_LastFrameTime = currentFrameTime;
	}

	return deltatime;
}

void Application::ProcessInput()
{
	// forward input data to ImGui first
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse || io.WantCaptureKeyboard)
		return;

	if (Input::IsMouseButtonPressed(Mouse::BUTTON_1))
	{
		// hide cursor when moving camera
		m_Window->HideCursor();
	}
	else if (Input::IsMouseButtonReleased(Mouse::BUTTON_1))
	{
		// unhide cursor when camera stops moving
		m_Window->ShowCursor();
	}
}

void Application::OnCloseEvent()
{
	m_IsRunning = false;
}

void Application::OnResizeEvent(int width, int height)
{
	m_Renderer->OnResize(width, height);
}

void Application::OnMouseMoveEvent(double xpos, double ypos)
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;

	m_Renderer->OnMouseMove(xpos, ypos);
}

void Application::OnMouseButtonEvent(int button, int action, int mods)
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;
}

void Application::OnMouseScrollEvent(double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;
}

void Application::OnKeyEvent(int key, int scancode, int action, int mods)
{
	// quits the application
	// works even when the ui is in focus
	if (Input::IsKeyPressed(Key::LEFT_CONTROL) && Input::IsKeyPressed(Key::Q))
		m_IsRunning = false;

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard)
		return;
}