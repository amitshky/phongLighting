#pragma once

#include <functional>

#include <GLFW/glfw3.h>


struct WindowProps
{
public:
	const char* title;
	uint32_t width;
	uint32_t height;

public:
	WindowProps(const char* title, uint32_t width = 1280, uint32_t height = 720)
		: title{ title },
		  width{ width },
		  height{ height }
	{}
};

class Window
{
public:
	using ResizeEventCallbackFn = std::function<void(int width, int height)>;
	using MouseEventCallbackFn = std::function<void(double xpos, double ypos)>;
	using KeyEventCallbackFn = std::function<void(int key, int scancode, int action, int mods)>;

public:
	Window(const WindowProps& props);
	~Window();

	inline GLFWwindow* GetWindowContext() const { return m_Window; }

	inline bool IsClosed() { return glfwWindowShouldClose(m_Window); }
	inline void Close() { glfwSetWindowShouldClose(m_Window, true); }

	inline void OnUpdate() { glfwPollEvents(); }

	inline void SetResizeEventCallbackFn(const ResizeEventCallbackFn& callback) { m_Data.ResizeEventCallback = callback; }
	inline void SetMouseEventCallbackFn(const MouseEventCallbackFn& callback) { m_Data.MouseEventCallback = callback; }
	inline void SetKeyEventCallbackFn(const KeyEventCallbackFn& callback) { m_Data.KeyEventCallback = callback; }

private:
	void Init(const WindowProps& props);

private:
	struct WindowData
	{
		const char* title;
		uint32_t width;
		uint32_t height;

		ResizeEventCallbackFn ResizeEventCallback = [](int width, int height) {};
		MouseEventCallbackFn MouseEventCallback = [](double xpos, double ypos) {};
		KeyEventCallbackFn KeyEventCallback = [](int key, int scancode, int action, int mods) {};
	};

	WindowData m_Data;
	GLFWwindow* m_Window;
};
