#pragma once
#include "D3DCore/Core/Layer.h"

namespace d3dcore
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& event) override;

		void Begin();
		void End();

		inline void BlockEvents(bool block) { m_blockEvents = block; }
		bool IsBlockingEvents() const { return m_blockEvents; }

	private:
		bool m_blockEvents = true;
	};
}