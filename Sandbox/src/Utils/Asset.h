#pragma once

#include "D3DCore.h"
#include "GlobalAsset.h"
#include "ShaderCBufs.h"
#include "D3DCore/Utils/BasicMesh.h"

class Asset
{
public:
	inline static void LoadAssets()
	{
		LoadShaders();
		LoadTextures();
		LoadBuffers();
	}

private:
	inline static void LoadShaders()
	{
		GlobalAsset::CreateAndAddShader("basic", "res/shaders/basic.vs.hlsl", "res/shaders/basic.ps.hlsl");
		GlobalAsset::CreateAndAddShader("lighting", "res/shaders/light.vs.hlsl", "res/shaders/light.ps.hlsl");
		GlobalAsset::CreateAndAddShader("nullps", "res/shaders/basic.vs.hlsl");
		GlobalAsset::CreateAndAddShader("fullscreen_invert", "res/shaders/fullscreen.vs.hlsl", "res/shaders/fullscreen_invert.ps.hlsl");
		GlobalAsset::CreateAndAddShader("fullscreen_blur", "res/shaders/fullscreen.vs.hlsl", "res/shaders/fullscreen_blur.ps.hlsl");
	}

	inline static void LoadTextures()
	{
		d3dcore::Texture2DDesc defTexDesc = {};
		defTexDesc.width = 1;
		defTexDesc.height = 1;
		uint32_t white = 0xffffffff;
		GlobalAsset::CreateAndAddTexture("default", &white, defTexDesc);
	}

	inline static void LoadBuffers()
	{
		using namespace d3dcore;

		float screenVertices[] = {
			-1.0f,  1.0f,
			 1.0f,  1.0f,
			 1.0f, -1.0f,
			-1.0f, -1.0f,
		};

		uint32_t screenIndices[] = {
			0, 1, 2,
			2, 3, 0
		};

		VertexBufferDesc screenVBDesc = {};
		screenVBDesc.size = uint32_t(2 * 4 * sizeof(float));
		screenVBDesc.stride = 2 * sizeof(float);
		screenVBDesc.usage = D3D11_USAGE_DEFAULT;
		screenVBDesc.cpuAccessFlag = 0;
		GlobalAsset::CreateAndAddVertexBuffer("screen_vb", screenVertices, screenVBDesc);

		IndexBufferDesc screenIBDesc = {};
		screenIBDesc.count = 6;
		screenIBDesc.usage = D3D11_USAGE_DEFAULT;
		screenIBDesc.cpuAccessFlag = 0;
		GlobalAsset::CreateAndAddIndexBuffer("screen_ib", screenIndices, screenIBDesc);



		auto cubeVertices = utils::CreateCubeVerticesEx();
		auto cubeIndices = utils::CreateCubeIndicesEx();
		auto planeVertices = utils::CreatePlaneVerticesEx();
		auto planeIndices = utils::CreatePlaneIndicesEx();

		VertexBufferDesc cubeVBDesc = {};
		cubeVBDesc.size = (uint32_t)cubeVertices.size() * sizeof(utils::Vertex);
		cubeVBDesc.stride = sizeof(utils::Vertex);
		cubeVBDesc.usage = D3D11_USAGE_DEFAULT;
		cubeVBDesc.cpuAccessFlag = 0;
		GlobalAsset::CreateAndAddVertexBuffer("cube_vb", cubeVertices.data(), cubeVBDesc);

		IndexBufferDesc cubeIBDesc = {};
		cubeIBDesc.count = (uint32_t)cubeIndices.size();
		cubeIBDesc.usage = D3D11_USAGE_DEFAULT;
		cubeIBDesc.cpuAccessFlag = 0;
		GlobalAsset::CreateAndAddIndexBuffer("cube_ib", cubeIndices.data(), cubeIBDesc);

		VertexBufferDesc planeVBDesc = {};
		planeVBDesc.size = (uint32_t)planeVertices.size() * sizeof(utils::Vertex);
		planeVBDesc.stride = sizeof(utils::Vertex);
		planeVBDesc.usage = D3D11_USAGE_DEFAULT;
		planeVBDesc.cpuAccessFlag = 0;
		GlobalAsset::CreateAndAddVertexBuffer("plane_vb", planeVertices.data(), planeVBDesc);

		IndexBufferDesc planeIBDesc = {};
		planeIBDesc.count = (uint32_t)planeIndices.size();
		planeIBDesc.usage = D3D11_USAGE_DEFAULT;
		planeIBDesc.cpuAccessFlag = 0;
		GlobalAsset::CreateAndAddIndexBuffer("plane_ib", planeIndices.data(), planeIBDesc);




		//std::cout << sizeof(cbufs::light::VSSystemCBuf) << '\n';
		//std::cout << sizeof(cbufs::light::VSEntityCBuf)	<< '\n';
		//std::cout << sizeof(cbufs::light::PSSystemCbuf)	<< '\n';
		//std::cout << sizeof(cbufs::light::PSEntityCBuf)	<< '\n';
		
		//std::cout << sizeof(cbufs::basic::VSSystemCBuf) << '\n';
		//std::cout << sizeof(cbufs::basic::VSEntityCBuf) << '\n';
		//std::cout << sizeof(cbufs::basic::PSEntityCBuf) << '\n';

		GlobalAsset::CreateAndAddCBuf("light_vs_system", sizeof(cbufs::light::VSSystemCBuf));
		GlobalAsset::CreateAndAddCBuf("light_vs_entity", sizeof(cbufs::light::VSEntityCBuf));
		GlobalAsset::CreateAndAddCBuf("light_ps_system", sizeof(cbufs::light::PSSystemCbuf));
		GlobalAsset::CreateAndAddCBuf("light_ps_entity", sizeof(cbufs::light::PSEntityCBuf));

		GlobalAsset::CreateAndAddCBuf("basic_vs_system", sizeof(cbufs::basic::VSSystemCBuf));
		GlobalAsset::CreateAndAddCBuf("basic_vs_entity", sizeof(cbufs::basic::VSEntityCBuf));
		GlobalAsset::CreateAndAddCBuf("basic_ps_entity", sizeof(cbufs::basic::PSEntityCBuf));
	}
};