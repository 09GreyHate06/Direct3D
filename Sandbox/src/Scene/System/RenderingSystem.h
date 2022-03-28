#pragma once
#include "D3DCore.h"

class RenderingSystem : public d3dcore::System
{
public:
	RenderingSystem(d3dcore::Scene* scene);
	RenderingSystem();

	void Render(const d3dcore::utils::Camera& camera);
	const std::shared_ptr<d3dcore::Framebuffer> GetFramebuffer() const { return m_framebuffer; }

private:
	void Render_(const d3dcore::utils::Camera& camera);
	//void Render_Outlined(const d3dcore::utils::Camera& camera);

	void SetLigths();
	DirectX::XMMATRIX GetEntityParentsTransform(d3dcore::RelationshipComponent& relationship);

	std::shared_ptr<d3dcore::Shader> m_lightShader;
	std::shared_ptr<d3dcore::Shader> m_textureShader;

	std::shared_ptr<d3dcore::ConstantBuffer> m_lightVSSysCBuf;
	std::shared_ptr<d3dcore::ConstantBuffer> m_lightVSEntityCBuf;
	std::shared_ptr<d3dcore::ConstantBuffer> m_lightPSSysCBuf;
	std::shared_ptr<d3dcore::ConstantBuffer> m_lightPSEntityCBuf;

	std::shared_ptr<d3dcore::ConstantBuffer> m_textureVSSysCBuf;
	std::shared_ptr<d3dcore::ConstantBuffer> m_textureVSEntityCBuf;
	std::shared_ptr<d3dcore::ConstantBuffer> m_texturePSEntityCBuf;

	std::shared_ptr<d3dcore::Texture2D> m_defaultTexture;
	std::shared_ptr<d3dcore::Framebuffer> m_framebuffer;
};