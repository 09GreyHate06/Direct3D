#pragma once
#include "Base.h"

#ifdef D3DC_ENABLE_ASSERTS
#define D3DC_CORE_ASSERT(x, ...) { if(!(x)) { D3DC_CORE_LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define D3DC_ASSERT(x, ...) { if(!(x)) { D3DC_LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define D3DC_CORE_ASSERT(x, ...)
#define D3DC_ASSERT(x, ...)
#endif // D3DC_ENABLE_ASSERTS
