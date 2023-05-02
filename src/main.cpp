#include <iostream>
#include <memory>

#include "core/core.h"
#include "core/window.h"
#include "core/input.h"
#include "core/KeyCodes.h"
#include "core/MouseButtonCodes.h"


void OnResizeEvent(int width, int height);
void OnMouseEvent(double xpos, double ypos);
void OnKeyEvent(int key, int scancode, int action, int mods);


int main()
{
	std::unique_ptr<Window> window{ std::make_unique<Window>(WindowProps{ "Phong Lighting" }) };
	window->SetResizeEventCallbackFn(OnResizeEvent);
	window->SetMouseEventCallbackFn(OnMouseEvent);
	window->SetKeyEventCallbackFn(OnKeyEvent);

	while (!window->IsClosed())
	{
		window->OnUpdate();
	}
}


void OnResizeEvent(int width, int height)
{}

void OnMouseEvent(double xpos, double ypos)
{
	std::cout << "x: " << xpos << " y: " << ypos << '\n';
};

void OnKeyEvent(int key, int scancode, int action, int mods)
{
	if (key == KEY_ESCAPE && action == GLFW_PRESS)
		std::cout << "esc pressed\n";
}