#pragma once

#include <memory>

#include "core/window.h"


class Application
{
public:
	Application(const char* title, uint32_t width = 1280, uint32_t height = 720);

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void Run();

	static inline Application* Create(const char* title) { return new Application{ title }; }
	static inline Application& GetInstance() { return *s_Instance; }

	inline Window& GetWindow() const { return *m_Window; }

private:
	void Init();

	void OnCloseEvent();
	void OnResizeEvent(int width, int height);
	void OnMouseMoveEvent(double xpos, double ypos);
	void OnMouseButtonEvent(int button, int action, int mods);
	void OnMouseScrollEvent(double xoffset, double yoffset);
	void OnKeyEvent(int key, int scancode, int action, int mods);

private:
	bool m_IsRunning = true;
	static Application* s_Instance;
	std::unique_ptr<Window> m_Window;
};