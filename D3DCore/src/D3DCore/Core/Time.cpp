#include "d3dcpch.h"
#include "Time.h"

using namespace std::chrono;

namespace d3dcore
{
	steady_clock::time_point Time::s_startTime;
	steady_clock::time_point Time::s_lastFrame;
	float Time::s_deltaBuffer = 0;

	float Time::GetTime()
	{
		auto currentTime = high_resolution_clock::now();
		return duration<float>(currentTime - s_startTime).count();
	}

	void Time::Init()
	{
		s_startTime = high_resolution_clock::now();
		s_lastFrame = steady_clock::now();
	}

	void Time::UpdateDelta()
	{
		const auto old = s_lastFrame;
		s_lastFrame = steady_clock::now();
		const duration<float> frametime = s_lastFrame - old;
		s_deltaBuffer = frametime.count();
	}

	Timer::Timer()
	{
		m_last = steady_clock::now();
	}

	float Timer::Mark()
	{
		const auto old = m_last;
		m_last = steady_clock::now();
		const duration<float> frametime = m_last - old;
		return frametime.count();
	}

	float Timer::Peek() const
	{
		return duration<float>(steady_clock::now() - m_last).count();
	}

}