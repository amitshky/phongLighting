#include "core/input.h"

#include <GLFW/glfw3.h>
#include "core/application.h"


bool Input::IsKeyPressed(int keycode)
{
	GLFWwindow* window = Application::GetInstance().GetWindow().GetWindowHandle();
	auto status = glfwGetKey(window, keycode);
	return status == GLFW_PRESS || status == GLFW_REPEAT;
}

bool Input::IsKeyReleased(int keycode)
{
	GLFWwindow* window = Application::GetInstance().GetWindow().GetWindowHandle();
	auto status = glfwGetKey(window, keycode);
	return status == GLFW_RELEASE;
}

bool Input::IsMouseButtonPressed(int button)
{
	GLFWwindow* window = Application::GetInstance().GetWindow().GetWindowHandle();
	auto status = glfwGetMouseButton(window, button);
	return status == GLFW_PRESS;
}

bool Input::IsMouseButtonReleased(int button)
{
	GLFWwindow* window = Application::GetInstance().GetWindow().GetWindowHandle();
	auto status = glfwGetMouseButton(window, button);
	return status == GLFW_RELEASE;
}

std::pair<float, float> Input::GetMousePosition()
{
	GLFWwindow* window = Application::GetInstance().GetWindow().GetWindowHandle();
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	return { static_cast<float>(xpos), static_cast<float>(ypos) };
}

float Input::GetMouseX()
{
	auto [x, y] = Input::GetMousePosition();
	return x;
}

float Input::GetMouseY()
{
	auto [x, y] = Input::GetMousePosition();
	return y;
}
