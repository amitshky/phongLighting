#pragma once

#include <memory>
#include <chrono>
#include "core/window.h"
#include "renderer/renderer.h"


class Application
{
public:
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	static Application* Create(const char* title);
	void Run();

	static inline Application& GetInstance() { return *s_Instance; }
	[[nodiscard]] inline Window& GetWindow() const { return *m_Window; }

private:
	explicit Application(const char* title, uint32_t width = 1280, uint32_t height = 720);

	void Init(const char* title);
	float CalcDeltaTime(); // deltatime in milliseconds

	// TODO: refactor to use OnEvent()
	void ProcessInput();
	void OnCloseEvent();
	void OnResizeEvent(int width, int height);
	void OnMouseMoveEvent(double xpos, double ypos);
	void OnMouseButtonEvent(int button, int action, int mods);
	void OnMouseScrollEvent(double xoffset, double yoffset);
	void OnKeyEvent(int key, int scancode, int action, int mods);

private:
	bool m_IsRunning = true;
	static Application* s_Instance;
	std::shared_ptr<Window> m_Window;
	std::unique_ptr<Renderer> m_Renderer;

	uint32_t m_LastFPS = 0;
	uint32_t m_FrameCounter = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_LastFrameTime{};
};