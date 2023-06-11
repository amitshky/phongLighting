#include "core/application.h"

#include <chrono>
#include "core/core.h"
#include "core/input.h"


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
	static auto lastFrameTime = std::chrono::high_resolution_clock::now();

	while (m_IsRunning)
	{
		auto currentFrameTime = std::chrono::high_resolution_clock::now();
		float deltatime =
			std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - lastFrameTime).count();
		lastFrameTime = currentFrameTime;

		// printf("\r%8d", static_cast<int32_t>(1 / deltatime));

		m_Renderer->Draw(deltatime);
		m_Window->OnUpdate();
		ProcessInput();
	}
}

void Application::ProcessInput()
{
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
	m_Renderer->OnMouseMove(xpos, ypos);
}

void Application::OnMouseButtonEvent(int /*unused*/, int /*unused*/, int /*unused*/)
{}

void Application::OnMouseScrollEvent(double /*unused*/, double /*unused*/)
{}

void Application::OnKeyEvent(int /*unused*/, int /*unused*/, int /*unused*/, int /*unused*/)
{
	if (Input::IsKeyPressed(Key::ESCAPE))
		m_IsRunning = false;
}