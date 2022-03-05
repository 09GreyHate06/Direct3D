#pragma once
#include "Event.h"
#include "D3DCore/Core/KeyCodes.h"
#include <sstream>

namespace d3dcore
{
	class KeyEvent : public Event
	{
	public:
		KeyEvent(const KeyCode keycode)
			: m_keyCode(keycode)
		{
		}

		KeyCode GetKeyCode() const { return m_keyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyCode m_keyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const KeyCode keycode, const int repeatCount)
			: KeyEvent(keycode), m_repeatCount(repeatCount) { }

		uint16_t GetRepeatCount() const { return m_repeatCount; }

		std::string ToString() const override
		{
			std::ostringstream oss;
			oss << "KeyPressedEvent: " << m_keyCode << " (" << m_repeatCount << " repeats)";
			return oss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		uint16_t m_repeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		using KeyEvent::KeyEvent;

		virtual std::string ToString() const override
		{
			std::ostringstream oss;
			oss << "KeyReleasedEvent: " << m_keyCode;
			return oss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		using KeyEvent::KeyEvent;

		virtual std::string ToString() const override
		{
			std::ostringstream oss;
			oss << "KeyTypedEvent: " << m_keyCode;
			return oss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}