#include <D3DCore.h>
#include <D3DCore/Core/EntryPoint.h>

class ExampleLayer : public d3dcore::Layer
{
public:
	virtual void OnEvent(d3dcore::Event& event) override
	{
		if (event.GetEventType() == d3dcore::EventType::KeyTyped)
		{
			d3dcore::KeyTypedEvent& e = static_cast<d3dcore::KeyTypedEvent&>(event);
			D3DC_LOG_TRACE(static_cast<char>(e.GetKeyCode()));
		}
	}
};

class Sandbox : public d3dcore::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
};

d3dcore::Application* d3dcore::CreateApplication()
{
	return new Sandbox();
}