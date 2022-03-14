#pragma once
#include <chrono>

namespace d3dcore
{
	class Time
	{
	public:
		static float GetTime();
		static float Delta() { return s_deltaBuffer; };

	private:
		static void Init();
		static void UpdateDelta();

		static std::chrono::steady_clock::time_point s_startTime;
		static std::chrono::steady_clock::time_point s_lastFrame;
		static float s_deltaBuffer;

		friend class Application;
	};

	class Timer
	{
	public:
		Timer();
		float Mark();
		float Peek() const;

	private:
		std::chrono::steady_clock::time_point m_last;
	};
}