#include "core/window.h"

#include "core/core.h"


Window::Window(const WindowProps& props)
{
	Init(props);
}

Window::~Window()
{
	glfwDestroyWindow(m_WindowHandle);
	glfwTerminate();
}

void Window::Init(const WindowProps& props)
{
	m_Data.title = props.title;
	m_Data.width = props.width;
	m_Data.height = props.height;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_WindowHandle =
		glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), props.title, nullptr, nullptr);
	glfwMakeContextCurrent(m_WindowHandle);

	glfwSetWindowUserPointer(m_WindowHandle, reinterpret_cast<void*>(&m_Data));

	// register GLFW event callbacks
	// window event callbacks
	glfwSetWindowCloseCallback(m_WindowHandle, [](GLFWwindow* window) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->CloseEventCallback();
	});

	glfwSetWindowSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->width = width;
		data->height = height;
		data->ResizeEventCallback(width, height);
	});

	// mouse event callbacks
	glfwSetCursorPosCallback(m_WindowHandle, [](GLFWwindow* window, double xpos, double ypos) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->MouseEventCallback(xpos, ypos);
	});

	glfwSetMouseButtonCallback(m_WindowHandle, [](GLFWwindow* window, int button, int action, int mods) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->MouseButtonCallback(button, action, mods);
	});

	glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow* window, double xoffset, double yoffset) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
		data->MouseScrollCallback(xoffset, yoffset);
	});

	// key event callbacks
	glfwSetKeyCallback(m_WindowHandle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
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
	THROW(glfwCreateWindowSurface(vulkanInstance, m_WindowHandle, nullptr, &m_WindowSurface) != VK_SUCCESS,
		"Failed to create window surface!")
}

bool Window::IsMinimized()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_WindowHandle, &width, &height);
	return width == 0 || height == 0;
}
