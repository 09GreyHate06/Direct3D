#include "EditorLayer.h"
#include "Scene/Components/Components.h"
#include "Utils/ModelLoader.h"
#include "Utils/FileDialog.h"

using namespace d3dcore;
using namespace DirectX;
using namespace Microsoft::WRL;

void EditorLayer::OnAttach()
{
	auto& app = Application::Get();
	utils::CameraDesc camDesc = {};
	camDesc.aspect = static_cast<float>(app.GetWindow().GetWidth()) / app.GetWindow().GetHeight();
	camDesc.fov = 45.0f;
	camDesc.nearPlane = 0.1f;
	camDesc.farPlane = 1000.0f;
	camDesc.position = { 0.0f, 0.0f, -5.0f };
	camDesc.yaw = 0.0f;
	camDesc.pitch = 0.0f;
	m_camera.Set(camDesc);
	m_editorCamController.SetContext(&m_camera);

	m_scene = std::make_unique<Scene>();
	m_renderingSystem = std::make_unique<RenderingSystem>(m_scene.get());
	m_sceneHierarchyPanel.SetContext(m_scene.get());

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	Renderer::SetDepthStencilState(dsDesc);

	Entity dirLight = m_scene->CreateEntity("Directional Light");

	dirLight.GetComponent<TransformComponent>().rotation = { 50.0f, -30.0f, 0.0f };
	auto& light = dirLight.AddComponent<DirectionalLightComponent>();
	light.color = { 1.0f, 1.0f, 1.0f };
	light.ambientIntensity = 0.2f;
	light.diffuseIntensity = 0.8f;
	light.specularIntensity = 1.0f;


	auto cubeVertices = utils::CreateCubeVerticesEx();
	auto cubeIndices = utils::CreateCubeIndicesEx();
	auto planeVertices = utils::CreatePlaneVerticesEx();
	auto planeIndices = utils::CreatePlaneIndicesEx();

	VertexBufferDesc cubeVBDesc = {};
	cubeVBDesc.size = (uint32_t)cubeVertices.size() * sizeof(utils::Vertex);
	cubeVBDesc.stride = sizeof(utils::Vertex);
	cubeVBDesc.usage = D3D11_USAGE_DEFAULT;
	cubeVBDesc.cpuAccessFlag = 0;
	m_cubeVB = VertexBuffer::Create(cubeVertices.data(), cubeVBDesc);

	IndexBufferDesc cubeIBDesc = {};
	cubeIBDesc.count = (uint32_t)cubeIndices.size();
	cubeIBDesc.usage = D3D11_USAGE_DEFAULT;
	cubeIBDesc.cpuAccessFlag = 0;
	m_cubeIB = IndexBuffer::Create(cubeIndices.data(), cubeIBDesc);

	VertexBufferDesc planeVBDesc = {};
	planeVBDesc.size = (uint32_t)planeVertices.size() * sizeof(utils::Vertex);
	planeVBDesc.stride = sizeof(utils::Vertex);
	planeVBDesc.usage = D3D11_USAGE_DEFAULT;
	planeVBDesc.cpuAccessFlag = 0;
	m_planeVB = VertexBuffer::Create(planeVertices.data(), planeVBDesc);

	IndexBufferDesc planeIBDesc = {};
	planeIBDesc.count = (uint32_t)planeIndices.size();
	planeIBDesc.usage = D3D11_USAGE_DEFAULT;
	planeIBDesc.cpuAccessFlag = 0;
	m_planeIB = IndexBuffer::Create(planeIndices.data(), planeIBDesc);
}

void EditorLayer::OnEvent(d3dcore::Event& event)
{
	if (event.GetEventType() == EventType::WindowResize)
	{
		auto& e = static_cast<WindowResizeEvent&>(event);
		if (e.GetWidth() > 0 && e.GetHeight() > 0)
		{
			ViewportDesc vpDesc = {};
			vpDesc.width = static_cast<float>(e.GetWidth());
			vpDesc.height = static_cast<float>(e.GetHeight());
			vpDesc.topLeftX = 0.0f;
			vpDesc.topLeftY = 0.0f;
			vpDesc.minDepth = 0.0f;
			vpDesc.maxDepth = 1.0f;
			Renderer::SetViewport(vpDesc);
		}
	}

	m_editorCamController.OnEvent(event);
}

void EditorLayer::OnUpdate()
{
	if (m_viewportFocus)
		m_editorCamController.OnUpdate();
	else
		m_editorCamController.Reset();

	if (const FramebufferDesc& desc = m_renderingSystem->GetFramebuffer()->GetDesc();
		m_sceneViewportWidth > 0.0f && m_sceneViewportHeight > 0.0f &&
		(desc.width != m_sceneViewportWidth || desc.height != m_sceneViewportHeight))
	{
		m_renderingSystem->GetFramebuffer()->Resize(static_cast<uint32_t>(m_sceneViewportWidth), static_cast<uint32_t>(m_sceneViewportHeight));
		m_editorCamController.SetViewportSize(m_sceneViewportWidth, m_sceneViewportHeight);
	}

	m_renderingSystem->Render(m_camera);
}

void EditorLayer::OnDetach()
{
}

void EditorLayer::OnImGuiRender()
{
	static bool openDockspace = true;
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
			if (ImGui::MenuItem("Create Cube"))
				CreateCube();

			if (ImGui::MenuItem("Create Plane"))
				CreatePlane();

			if (ImGui::MenuItem("Load Model"))
			{
				std::string filename = FileDialog::OpenFileDialog("3D Model Files (*.obj, *.fbx)\0*.obj;*.fbx\0");
				if (!filename.empty())
					ModelLoader::LoadModel(filename, m_scene.get());
			}

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

void EditorLayer::CreateCube()
{
	Entity entity = m_scene->CreateEntity("Cube");
	auto& mc = entity.AddComponent<MeshComponent>();
	mc.vBuffer = m_cubeVB;
	mc.iBuffer = m_cubeIB;
	auto& mat = entity.AddComponent<MaterialComponent>();
	mat.diffuseMap = nullptr;
	mat.specularMap = nullptr;
	mat.ambientCol = { 1.0f, 1.0f, 1.0f };
	mat.diffuseCol = { 1.0f, 1.0f, 1.0f };
	mat.specularCol = { 1.0f, 1.0f, 1.0f };
	mat.tiling = { 1.0f, 1.0f };
	mat.shininess = 32.0f;
	entity.AddComponent<MeshRendererComponent>();
}

void EditorLayer::CreatePlane()
{
	Entity entity = m_scene->CreateEntity("Plane");
	auto& mc = entity.AddComponent<MeshComponent>();
	mc.vBuffer = m_planeVB;
	mc.iBuffer = m_planeIB;
	auto& mat = entity.AddComponent<MaterialComponent>();
	mat.diffuseMap = nullptr;
	mat.specularMap = nullptr;
	mat.ambientCol = { 1.0f, 1.0f, 1.0f };
	mat.diffuseCol = { 1.0f, 1.0f, 1.0f };
	mat.specularCol = { 1.0f, 1.0f, 1.0f };
	mat.tiling = { 1.0f, 1.0f };
	mat.shininess = 32.0f;
	entity.AddComponent<MeshRendererComponent>();
}
