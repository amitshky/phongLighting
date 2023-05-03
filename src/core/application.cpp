#include "application.h"

#include <fmt/core.h>

#include "core/core.h"
#include "core/keyCodes.h"


Application::Application(const char* title, uint32_t width, uint32_t height)
	: m_Window{ std::make_unique<Window>(WindowProps{ title }) }
{
	Init();
}

void Application::Init()
{
	m_Window->SetCloseEventCallbackFn(BIND_EVENT_FN(Application::OnCloseEvent));
	m_Window->SetResizeEventCallbackFn(BIND_EVENT_FN(Application::OnResizeEvent));
	m_Window->SetMouseEventCallbackFn(BIND_EVENT_FN(Application::OnMouseEvent));
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

void Application::OnMouseEvent(double xpos, double ypos)
{
	fmt::print("x: {}, y: {}\n", xpos, ypos);
}

void Application::OnKeyEvent(int key, int scancode, int action, int mods)
{
	if (key == KEY_ESCAPE && action == GLFW_PRESS)
		m_IsRunning = false;
}