#include "window.h"

#include <stdexcept>


Window::Window(const WindowProps& props)
{
	Init(props);
}

Window::~Window()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Window::Init(const WindowProps& props)
{
	m_Data.title = props.title;
	m_Data.width = props.width;
	m_Data.height = props.height;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), props.title, nullptr, nullptr);
	glfwMakeContextCurrent(m_Window);

	glfwSetWindowUserPointer(m_Window, reinterpret_cast<void*>(&m_Data));

	// register GLFW event callbacks
	glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->CloseEventCallback();
	});

	glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->ResizeEventCallback(width, height);
	});

	glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->MouseEventCallback(xpos, ypos);
	});

	glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->KeyEventCallback(key, scancode, action, mods);
	});
}
