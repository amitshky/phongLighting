#pragma once

#include <utility>
#include <stdexcept>

#include "core/logger.h"

#define GLFW_INCLUDE_VULKAN

// non-static member functions cannot be used inside the glfw callbacks, so we bind them to another function
#define BIND_EVENT_FN(fn) \
	[this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define THROW(condition, msg)          \
	if (condition)                     \
	{                                  \
		Logger::Error(msg);            \
		throw std::runtime_error(msg); \
	}