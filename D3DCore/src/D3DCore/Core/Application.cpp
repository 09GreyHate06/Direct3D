#include "d3dcpch.h"
#include "Application.h"
#include "Base.h"
#include "Time.h"

#include "D3DCore/Renderer/Renderer.h"

namespace d3dcore
{
	Application* Application::s_instance = nullptr;

	Application::Application()
	{
		D3DC_CORE_ASSERT(!s_instance, "Application already exists!");

		s_instance = this;

		m_window = std::make_unique<Window>(1280, 720, "D3DCore");
		m_window->SetEventCallback(D3DC_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_imGuiLayer = new ImGuiLayer();
		PushOverlay(m_imGuiLayer);
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_layerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_layerStack.PushOverlay(overlay);
	}

	void Application::PopLayer(Layer* layer)
	{
		m_layerStack.PopLayer(layer);
	}

	void Application::PopOverlay(Layer* overlay)
	{
		m_layerStack.PopOverlay(overlay);
	}

	void Application::Run()
	{
		Time::Init();

		while (m_running)
		{
			Time::UpdateDelta();

			for (Layer* layer : m_layerStack)
				layer->OnUpdate();

			if (!m_window->IsMinimized())
			{
				m_imGuiLayer->Begin();
				for (Layer* layer : m_layerStack)
					layer->OnImGuiRender();
				m_imGuiLayer->End();
			}

			Window::PollEvents();
			Renderer::SwapBuffers(1);
		}
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(D3DC_BIND_EVENT_FN(Application::OnWindowClosed));
		dispatcher.Dispatch<WindowResizeEvent>(D3DC_BIND_EVENT_FN(Application::OnWindowResized));

		for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it)
		{
			if (event.handled)
				break;
			(*it)->OnEvent(event);
		}
	}

	bool Application::OnWindowResized(WindowResizeEvent& event)
	{
		return false;
	}

	bool Application::OnWindowClosed(WindowCloseEvent& event)
	{
		m_running = false;
		return true;
	}

}