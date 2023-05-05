#pragma once

// disable "unreferenced formal parameter" warning for MSVC
// #pragma warning(disable : 4100)


#include <utility>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// non-static member functions cannot be used inside the glfw callbacks, so we
// bind them to another function
#define BIND_EVENT_FN(fn)                                       \
	[this](auto&&... args) -> decltype(auto) {                  \
		return this->fn(std::forward<decltype(args)>(args)...); \
	}