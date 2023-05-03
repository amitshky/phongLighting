#include "application.h"

#include <fmt/core.h>

#include "core/core.h"
#include "core/input.h"
#include "core/keyCodes.h"
#include "core/mouseButtonCodes.h"


Application* Application::s_Instance = nullptr;

Application::Application(const char* title, uint32_t width, uint32_t height)
	: m_Window{ std::make_unique<Window>(WindowProps{ title }) }
{
	Init();
}

void Application::Init()
{
	s_Instance = this;

	m_Window->SetCloseEventCallbackFn(BIND_EVENT_FN(Application::OnCloseEvent));
	m_Window->SetResizeEventCallbackFn(BIND_EVENT_FN(Application::OnResizeEvent));
	m_Window->SetMouseEventCallbackFn(BIND_EVENT_FN(Application::OnMouseMoveEvent));
	m_Window->SetMouseButtonCallbackFn(BIND_EVENT_FN(Application::OnMouseButtonEvent));
	m_Window->SetMouseScrollCallbackFn(BIND_EVENT_FN(Application::OnMouseScrollEvent));
	m_Window->SetKeyEventCallbackFn(BIND_EVENT_FN(Application::OnKeyEvent));
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
{
	fmt::print("Width: {}, Height: {}\n", width, height);
}

void Application::OnMouseMoveEvent(double xpos, double ypos)
{
	fmt::print("MouseX: {}, MouseY: {}\n", xpos, ypos);
}

void Application::OnMouseButtonEvent(int button, int action, int mods)
{
	if (Input::IsMouseButtonPressed(MOUSE_BUTTON_1))
		fmt::print("Left mouse button pressed\n");
}

void Application::OnMouseScrollEvent(double xoffset, double yoffset)
{
	fmt::print("ScrollX: {}, ScrollY: {}\n", xoffset, yoffset);
}

void Application::OnKeyEvent(int key, int scancode, int action, int mods)
{
	if (Input::IsKeyPressed(KEY_ESCAPE))
		m_IsRunning = false;
}