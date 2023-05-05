#include "core/window.h"

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

	m_Window = glfwCreateWindow(static_cast<int>(props.width),
		static_cast<int>(props.height),
		props.title,
		nullptr,
		nullptr);
	glfwMakeContextCurrent(m_Window);

	glfwSetWindowUserPointer(m_Window, reinterpret_cast<void*>(&m_Data));

	// register GLFW event callbacks
	// window event callbacks
	glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
		auto data =
			reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->CloseEventCallback();
	});

	glfwSetWindowSizeCallback(
		m_Window, [](GLFWwindow* window, int width, int height) {
			auto data =
				reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->ResizeEventCallback(width, height);
		});

	// mouse event callbacks
	glfwSetCursorPosCallback(
		m_Window, [](GLFWwindow* window, double xpos, double ypos) {
			auto data =
				reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->MouseEventCallback(xpos, ypos);
		});

	glfwSetMouseButtonCallback(
		m_Window, [](GLFWwindow* window, int button, int action, int mods) {
			auto data =
				reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->MouseButtonCallback(button, action, mods);
		});

	glfwSetScrollCallback(
		m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
			auto data =
				reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->MouseScrollCallback(xoffset, yoffset);
		});

	// key event callbacks
	glfwSetKeyCallback(m_Window,
		[](GLFWwindow* window, int key, int scancode, int action, int mods) {
			auto data =
				reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->KeyEventCallback(key, scancode, action, mods);
		});
}


void Window::OnUpdate()
{
	glfwPollEvents();
}

void Window::DestroyWindowSurface(VkInstance vulkanInstance)
{
	vkDestroySurfaceKHR(vulkanInstance, m_WindowSurface, nullptr);
}

void Window::CreateWindowSurface(VkInstance vulkanInstance)
{
	if (glfwCreateWindowSurface(
			vulkanInstance, m_Window, nullptr, &m_WindowSurface)
		!= VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface!");
}
