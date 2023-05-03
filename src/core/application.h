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

private:
	void Init();

	void OnCloseEvent();
	void OnResizeEvent(int width, int height);
	void OnMouseEvent(double xpos, double ypos);
	void OnKeyEvent(int key, int scancode, int action, int mods);

private:
	bool m_IsRunning = true;
	std::unique_ptr<Window> m_Window;
};