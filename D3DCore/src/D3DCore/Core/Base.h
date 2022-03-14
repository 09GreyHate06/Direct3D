#pragma once

#include <memory>

#ifdef D3DC_DEBUG
#define D3DC_ENABLE_ASSERTS
#endif // D3DC_DEBUG

#define D3DC_STRINGIFY(x) #x

#define BIT(x) (1 << x)

#define D3DC_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define D3DC_COM_SAFE_RELEASE(x) x->Release(); x = nullptr

#include "Log.h"
#include "D3DCAssert.h"