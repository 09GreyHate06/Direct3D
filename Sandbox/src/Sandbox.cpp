#include <D3DCore.h>
#include <D3DCore/Core/EntryPoint.h>

#include "Editor/EditorLayer.h"

class Sandbox : public d3dcore::Application
{
public:
	Sandbox()
	{
		PushLayer(new EditorLayer());
	}
};

d3dcore::Application* d3dcore::CreateApplication()
{
	return new Sandbox();
}