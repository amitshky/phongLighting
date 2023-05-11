#pragma once

#include <utility>
#include <stdexcept>

#include "core/logger.h"

#define GLFW_INCLUDE_VULKAN

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

// non-static member functions cannot be used inside the glfw callbacks, so we bind them to another function
#define BIND_EVENT_FN(fn) \
	[this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define THROW(condition, ...)                               \
	if (condition)                                          \
	{                                                       \
		Logger::Error(__VA_ARGS__);                         \
		throw std::runtime_error(fmt::format(__VA_ARGS__)); \
	}