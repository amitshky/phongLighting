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
	using CloseEventCallbackFn = std::function<void()>;
	using ResizeEventCallbackFn = std::function<void(int width, int height)>;
	using MouseEventCallbackFn = std::function<void(double xpos, double ypos)>;
	using KeyEventCallbackFn = std::function<void(int key, int scancode, int action, int mods)>;

public:
	Window(const WindowProps& props);
	~Window();

	inline GLFWwindow* GetWindowContext() const { return m_Window; }

	inline void OnUpdate() { glfwPollEvents(); }

	inline void SetCloseEventCallbackFn(const CloseEventCallbackFn& callback) { m_Data.CloseEventCallback = callback; }
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

		CloseEventCallbackFn CloseEventCallback;
		ResizeEventCallbackFn ResizeEventCallback;
		MouseEventCallbackFn MouseEventCallback;
		KeyEventCallbackFn KeyEventCallback;
	};

	WindowData m_Data;
	GLFWwindow* m_Window;
};
