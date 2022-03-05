#pragma once
#include "Base.h"

#include <spdlog/spdlog.h>

namespace d3dcore
{
	class Log
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_coreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_clientLogger; }

	private:
		Log() = default;
		
		static std::shared_ptr<spdlog::logger> s_coreLogger;
		static std::shared_ptr<spdlog::logger> s_clientLogger;
	};
}

// Core log macros
#define D3DC_CORE_LOG_TRACE(...)      d3dcore::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define D3DC_CORE_LOG_INFO(...)       d3dcore::Log::GetCoreLogger()->info(__VA_ARGS__)
#define D3DC_CORE_LOG_WARN(...)       d3dcore::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define D3DC_CORE_LOG_ERROR(...)      d3dcore::Log::GetCoreLogger()->error(__VA_ARGS__)
#define D3DC_CORE_LOG_CRITICAL(...)   d3dcore::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define D3DC_LOG_TRACE(...)           d3dcore::Log::GetClientLogger()->trace(__VA_ARGS__)
#define D3DC_LOG_INFO(...)            d3dcore::Log::GetClientLogger()->info(__VA_ARGS__)
#define D3DC_LOG_WARN(...)            d3dcore::Log::GetClientLogger()->warn(__VA_ARGS__)
#define D3DC_LOG_ERROR(...)           d3dcore::Log::GetClientLogger()->error(__VA_ARGS__)
#define D3DC_LOG_CRITICAL(...)        d3dcore::Log::GetClientLogger()->critical(__VA_ARGS__)