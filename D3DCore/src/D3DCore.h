#pragma once

#include "D3DCore/Core/D3DCWindows.h"
#include "D3DCore/Core/Window.h"
#include "D3DCore/Core/Application.h"
#include "D3DCore/Core/Layer.h"
#include "D3DCore/Core/Time.h"
#include "D3DCore/Core/KeyCodes.h"
#include "D3DCore/Core/MouseCodes.h"
#include "D3DCore/Core/Input.h"
#include "D3DCore/Core/Log.h"

#include "D3DCore/Events/ApplicationEvent.h"
#include "D3DCore/Events/Event.h"
#include "D3DCore/Events/KeyEvent.h"
#include "D3DCore/Events/MouseEvent.h"

#include "D3DCore/ImGui/ImGuiLayer.h"

#include "D3DCore/Renderer/Renderer.h"
#include "D3DCore/Renderer/Framebuffer.h"
#include "D3DCore/Renderer/Shader.h"
#include "D3DCore/Renderer/Buffer.h"
#include "D3DCore/Renderer/Texture.h"

#include "D3DCore/Scene/Components.h"
#include "D3DCore/Scene/Entity.h"
#include "D3DCore/Scene/Scene.h"
#include "D3DCore/Scene/System.h"

#include "D3DCore/Utils/BasicMesh.h"
#include "D3DCore/Utils/Loader.h"
#include "D3DCore/Utils/Camera.h"
#include "D3DCore/Utils/EditorCamController.h"
#include "D3DCore/Utils/FPSCamController.h"
#include "D3DCore/Utils/ShaderBuffer.h"

#include <entt/entt.hpp>
#include <d3d11.h>
#include <DirectXMath.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#ifdef D3DC_INTERNALS
#include "D3DCore/Renderer/D3DContext.h"
#endif // D3DC_INTERNAL