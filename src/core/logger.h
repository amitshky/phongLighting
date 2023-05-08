#pragma once

#include <memory>
#include "spdlog/spdlog.h"

class Logger
{
public:
	static void Init();

	template<typename... Args>
	static inline void Log(Args... args)
	{
		s_Logger->trace(args...);
	}

	template<typename... Args>
	static inline void Info(Args... args)
	{
		s_Logger->info(args...);
	}

	template<typename... Args>
	static inline void Warn(Args... args)
	{
		s_Logger->warn(args...);
	}

	template<typename... Args>
	static inline void Error(Args... args)
	{
		s_Logger->error(args...);
	}

	template<typename... Args>
	static inline void Critical(Args... args)
	{
		s_Logger->critical(args...);
	}

private:
	static std::shared_ptr<spdlog::logger> s_Logger;
};
