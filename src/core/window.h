#pragma once

#include <functional>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


struct WindowProps
{
public:
	const char* title;
	uint32_t width;
	uint32_t height;

public:
	explicit WindowProps(const char* title, uint32_t width = 1280, uint32_t height = 720)
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
	using MouseButtonCallbackFn = std::function<void(int button, int action, int mods)>;
	using MouseScrollCallbackFn = std::function<void(double xoffset, double yoffset)>;
	using KeyEventCallbackFn = std::function<void(int key, int scancode, int action, int mods)>;

public:
	explicit Window(const WindowProps& props);
	~Window();

	void OnUpdate();
	void DestroyWindowSurface(VkInstance vulkanInstance);
	void CreateWindowSurface(VkInstance vulkanInstance);
	bool IsMinimized();

	inline uint32_t GetWidth() const { return m_Data.width; }
	inline uint32_t GetHeight() const { return m_Data.height; }

	inline GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
	inline VkSurfaceKHR GetWindowSurface() const { return m_WindowSurface; }
	inline void GetFramebufferSize(int* width, int* height) const
	{
		glfwGetFramebufferSize(m_WindowHandle, width, height);
	}
	static inline const char** GetRequiredVulkanExtensions(uint32_t* count)
	{
		return glfwGetRequiredInstanceExtensions(count);
	}
	inline void WaitEvents() { glfwWaitEvents(); }
	inline void HideCursor() { glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
	inline void ShowCursor() { glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

	// set event callbacks
	inline void SetCloseEventCallbackFn(const CloseEventCallbackFn& callback) { m_Data.CloseEventCallback = callback; }
	inline void SetResizeEventCallbackFn(const ResizeEventCallbackFn& callback)
	{
		m_Data.ResizeEventCallback = callback;
	}

	inline void SetMouseEventCallbackFn(const MouseEventCallbackFn& callback) { m_Data.MouseEventCallback = callback; }
	inline void SetMouseButtonCallbackFn(const MouseButtonCallbackFn& callback)
	{
		m_Data.MouseButtonCallback = callback;
	}
	inline void SetMouseScrollCallbackFn(const MouseScrollCallbackFn& callback)
	{
		m_Data.MouseScrollCallback = callback;
	}

	inline void SetKeyEventCallbackFn(const KeyEventCallbackFn& callback) { m_Data.KeyEventCallback = callback; }

private:
	void Init(const WindowProps& props);

private:
	struct WindowData
	{
		const char* title;
		uint32_t width;
		uint32_t height;

		// event callback functions
		CloseEventCallbackFn CloseEventCallback = []() {};
		ResizeEventCallbackFn ResizeEventCallback = [](int, int) {};

		MouseEventCallbackFn MouseEventCallback = [](double, double) {};
		MouseButtonCallbackFn MouseButtonCallback = [](int, int, int) {};
		MouseScrollCallbackFn MouseScrollCallback = [](double, double) {};

		KeyEventCallbackFn KeyEventCallback = [](int, int, int, int) {};
	};

	WindowData m_Data;
	GLFWwindow* m_WindowHandle;

	VkSurfaceKHR m_WindowSurface;
};
