#pragma once
#include "LayerStack.h"
#include "Window.h"
#include "D3DCore/Events/ApplicationEvent.h"
#include "D3DCore/ImGui/ImGuiLayer.h"

namespace d3dcore
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		void Run();
		void OnEvent(Event& event);

		inline void Close() { m_window->Close(); }

		ImGuiLayer* GetImGuiLayer() const { return m_imGuiLayer; }
		Window& GetWindow() const { return *m_window; }
		static Application& Get() { return *s_instance; }

	private:
		bool OnWindowResized(WindowResizeEvent& event);
		bool OnWindowClosed(WindowCloseEvent& event);

		std::unique_ptr<Window> m_window;
		LayerStack m_layerStack;
		bool m_running = true;
		ImGuiLayer* m_imGuiLayer;
		
		static Application* s_instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}