#include "core/logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"


std::shared_ptr<spdlog::logger> Logger::s_Logger;

void Logger::Init()
{
	spdlog::set_pattern("%^[%T] %l: %v%$");
	s_Logger = spdlog::stdout_color_mt("phong_lighting_logger");
	s_Logger->set_level(spdlog::level::trace);
}