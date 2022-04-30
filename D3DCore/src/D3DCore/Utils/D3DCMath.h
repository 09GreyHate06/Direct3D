#pragma once
#include "Math.h"

namespace d3dcore::utils
{
	constexpr float PI = 3.14159265f;
	constexpr double PI_D = 3.1415926535897932;

	template<typename T>
	constexpr T Square(const T& x)
	{
		return x * x;
	}

	template<typename T>
	constexpr T Gauss(T x, T sigma)
	{
		const auto ss = Square(sigma);
		return ((T)1.0 / sqrt((T)2.0 * (T)PI_D * ss)) * exp(-Square(x) / ((T)2.0 * ss));
	}
}