#pragma once

#include <utility>
#include "core/keyCodes.h"
#include "core/mouseButtonCodes.h"

class Input
{
public:
	static bool IsKeyPressed(Key keycode);
	static bool IsKeyReleased(Key keycode);

	static bool IsMouseButtonPressed(Mouse button);
	static bool IsMouseButtonReleased(Mouse button);

	static std::pair<float, float> GetMousePosition();
	static float GetMouseX();
	static float GetMouseY();
};