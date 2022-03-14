#include "EditorLayer.h"
#include "Scene/Components/Components.h"
#include "Utils/ModelLoader.h"

using namespace d3dcore;
using namespace DirectX;
using namespace Microsoft::WRL;

void EditorLayer::OnAttach()
{
	m_camera.SetCamera(utils::EditorCameraDesc());
	m_scene = std::make_unique<Scene>();
	m_renderingSystem = std::make_unique<RenderingSystem>(m_scene.get());
	m_sceneHierarchyPanel.SetContext(m_scene.get());

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	Renderer::SetDepthStencilState(dsDesc);

	auto cubeVertices = utils::CreateCubeVertices();
	auto cubeIndices = utils::CreateCubeIndices();
	auto planeVertices = utils::CreatePlaneVertices(true, true, -0.5f, 0.5f, 10.0f, 10.0f);
	auto planeIndices = utils::CreatePlaneIndices();
	
	VertexBufferDesc cubeVbDesc = {};
	cubeVbDesc.data = cubeVertices.data();
	cubeVbDesc.size = static_cast<uint32_t>(cubeVertices.size()) * sizeof(float);
	cubeVbDesc.stride = 8 * sizeof(float);
	cubeVbDesc.usage = D3D11_USAGE_IMMUTABLE;
	cubeVbDesc.cpuAccessFlag = 0;
	auto cubeVB = VertexBuffer::Create(cubeVbDesc);

	IndexBufferDesc cubeIbDesc = {};
	cubeIbDesc.data = cubeIndices.data();
	cubeIbDesc.size = static_cast<uint32_t>(cubeIndices.size()) * sizeof(uint32_t);
	cubeIbDesc.usage = D3D11_USAGE_IMMUTABLE;
	cubeIbDesc.cpuAccessFlag = 0;
	auto cubeIB = IndexBuffer::Create(cubeIbDesc);

	VertexBufferDesc planeVbDesc = {};
	planeVbDesc.data = planeVertices.data();
	planeVbDesc.size = static_cast<uint32_t>(planeVertices.size()) * sizeof(float);
	planeVbDesc.stride = 8 * sizeof(float);
	planeVbDesc.usage = D3D11_USAGE_IMMUTABLE;
	planeVbDesc.cpuAccessFlag = 0;
	auto planeVB = VertexBuffer::Create(planeVbDesc);

	IndexBufferDesc planeIbDesc = {};
	planeIbDesc.data = planeIndices.data();
	planeIbDesc.size = static_cast<uint32_t>(planeIndices.size()) * sizeof(uint32_t);
	planeIbDesc.usage = D3D11_USAGE_IMMUTABLE;
	planeIbDesc.cpuAccessFlag = 0;
	auto planeIB = IndexBuffer::Create(planeIbDesc);


	auto steel1 = Texture2D::Create("res/textures/steel1.png", false, Texture2DDesc());
	auto groundMarble = Texture2D::Create("res/textures/groundMarble.png", false, Texture2DDesc());

	Entity cube = m_scene->CreateEntity();
	Entity plane = m_scene->CreateEntity();
	Entity dirLight = m_scene->CreateEntity();

	auto& cubeMesh = cube.AddComponent<MeshComponent>();
	cubeMesh.vBuffer = cubeVB;
	cubeMesh.iBuffer = cubeIB;
	auto& cubeMR = cube.AddComponent<MeshRendererComponent>();
	cubeMR.receiveLight = true;
	cubeMR.topology = Topology::TriangleList;
	auto& cubeMat = cube.AddComponent<MaterialComponent>();
	cubeMat.diffuseMap = steel1;
	cubeMat.specularMap = steel1;
	cubeMat.ambientCol = { 1.0f, 1.0f, 1.0f };
	cubeMat.diffuseCol = { 1.0f, 1.0f, 1.0f };
	cubeMat.specularCol = { 1.0f, 1.0f, 1.0f };
	cubeMat.shininess = 122.0f;

	auto& planeT = plane.GetComponent<TransformComponent>();
	planeT.position = { 0.0f, -1.0f, 0.0f };
	planeT.scale = { 30.0f, 30.0f, 30.0f };
	auto& planeMesh = plane.AddComponent<MeshComponent>();
	planeMesh.vBuffer = planeVB;
	planeMesh.iBuffer = planeIB;
	auto& planeMR = plane.AddComponent<MeshRendererComponent>();
	planeMR.receiveLight = true;
	planeMR.topology = Topology::TriangleList;
	auto& planeMat = plane.AddComponent<MaterialComponent>();
	planeMat.diffuseMap = groundMarble;
	planeMat.specularMap = groundMarble;
	planeMat.ambientCol = { 1.0f, 1.0f, 1.0f };
	planeMat.diffuseCol = { 1.0f, 1.0f, 1.0f };
	planeMat.specularCol = { 1.0f, 1.0f, 1.0f };
	planeMat.shininess = 32.0f;

	dirLight.GetComponent<TransformComponent>().rotation = { 50.0f, -30.0f, 0.0f };
	auto& light = dirLight.AddComponent<DirectionalLightComponent>();
	light.color = { 42.0f / 255.0f, 42.0f / 255.0f, 53.0f / 255.0f };
	light.ambientIntensity = 0.2f;
	light.diffuseIntensity = 0.8f;
	light.specularIntensity = 1.0f;
	
	ModelLoader::LoadModel("D:/3D_Models/Mech_F_432/mech_f_432.obj", m_scene.get());
}

void EditorLayer::OnEvent(d3dcore::Event& event)
{
	if (event.GetEventType() == EventType::WindowResize)
	{
		auto& e = static_cast<WindowResizeEvent&>(event);
		ViewportDesc vpDesc = {};
		vpDesc.width = static_cast<float>(e.GetWidth());
		vpDesc.height = static_cast<float>(e.GetHeight());
		vpDesc.topLeftX = 0.0f;
		vpDesc.topLeftY = 0.0f;
		vpDesc.minDepth = 0.0f;
		vpDesc.maxDepth = 1.0f;
		Renderer::SetViewport(vpDesc);
	}

	m_camera.OnEvent(event);
}

void EditorLayer::OnUpdate()
{
	if (m_viewportFocus)
		m_camera.OnUpdate();
	else
		m_camera.Reset();

	if (const FramebufferDesc& desc = m_renderingSystem->GetFramebuffer()->GetDesc();
		m_sceneViewportWidth > 0.0f && m_sceneViewportHeight > 0.0f &&
		(desc.width != m_sceneViewportWidth || desc.height != m_sceneViewportHeight))
	{
		m_renderingSystem->GetFramebuffer()->Resize(static_cast<uint32_t>(m_sceneViewportWidth), static_cast<uint32_t>(m_sceneViewportHeight));
		m_camera.SetViewportSize(m_sceneViewportWidth, m_sceneViewportHeight);
	}

	m_renderingSystem->Render(m_camera);
}

void EditorLayer::OnDetach()
{
}

void EditorLayer::OnImGuiRender()
{
	static bool openDockspace{ true };
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &openDockspace, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			//// Disabling fullscreen would allow the window to be moved to the front of other windows,
			//// which we can't undo at the moment without finer window depth/z control.
			//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
			//ImGui::MenuItem("Padding", NULL, &opt_padding);
			//ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				Application::Get().Close();

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::Begin("FPS");
	ImGui::Text(std::to_string(ImGui::GetIO().Framerate).c_str());
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
	ImGui::Begin("Viewport");
	m_viewportFocus = ImGui::IsWindowFocused() && ImGui::IsWindowHovered();
	Application::Get().GetImGuiLayer()->BlockEvents(!m_viewportFocus);
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	m_sceneViewportWidth = viewportPanelSize.x;
	m_sceneViewportHeight = viewportPanelSize.y;
	
	ImGui::Image(m_renderingSystem->GetFramebuffer()->GetView(), { m_sceneViewportWidth, m_sceneViewportHeight });
	ImGui::End();	
	ImGui::PopStyleVar();

	m_sceneHierarchyPanel.OnImGuiRender();
	ImGui::End();
}
