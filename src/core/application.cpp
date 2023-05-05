#include "core/application.h"

#include "core/core.h"
#include "core/input.h"
#include "core/keyCodes.h"
#include "core/mouseButtonCodes.h"


Application* Application::s_Instance = nullptr;

Application::Application(const char* title, uint32_t width, uint32_t height)
{
	Init(title);
}

void Application::Init(const char* title)
{
	s_Instance = this;

	m_Window = std::make_shared<Window>(WindowProps{ title });
	// set window event callbacks
	m_Window->SetCloseEventCallbackFn(BIND_EVENT_FN(Application::OnCloseEvent));
	m_Window->SetResizeEventCallbackFn(
		BIND_EVENT_FN(Application::OnResizeEvent));
	m_Window->SetMouseEventCallbackFn(
		BIND_EVENT_FN(Application::OnMouseMoveEvent));
	m_Window->SetMouseButtonCallbackFn(
		BIND_EVENT_FN(Application::OnMouseButtonEvent));
	m_Window->SetMouseScrollCallbackFn(
		BIND_EVENT_FN(Application::OnMouseScrollEvent));
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

void Application::Run()
{
	while (m_IsRunning)
	{
		m_Window->OnUpdate();
	}
}

void Application::OnCloseEvent()
{
	m_IsRunning = false;
}

void Application::OnResizeEvent(int width, int height)
{}

void Application::OnMouseMoveEvent(double xpos, double ypos)
{}

void Application::OnMouseButtonEvent(int button, int action, int mods)
{}

void Application::OnMouseScrollEvent(double xoffset, double yoffset)
{}

void Application::OnKeyEvent(int key, int scancode, int action, int mods)
{
	if (Input::IsKeyPressed(KEY_ESCAPE))
		m_IsRunning = false;
}