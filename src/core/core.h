#pragma once

#include <utility>
#include <stdexcept>

#include "core/logger.h"

// non-static member functions cannot be used inside the glfw callbacks, so we bind them to another function
#define BIND_EVENT_FN(fn) \
	[this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define LOG_AND_THROW(...)      \
	Logger::Error(__VA_ARGS__); \
	throw std::runtime_error(fmt::format(__VA_ARGS__));

#define THROW(condition, ...)       \
	if (condition)                  \
	{                               \
		LOG_AND_THROW(__VA_ARGS__); \
	}