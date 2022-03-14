#pragma once
#include <vector>
#include <array>
#include <DirectXMath.h>

namespace d3dcore::utils
{
	std::vector<float> CreateCubeVertices(bool hasTexCoords = true, bool hasNormals = true, float minPos = -0.5f, float maxPos = 0.5f, float texCoordU = 1.0f, float texCoordV = 1.0f);
	std::vector<float> CreatePlaneVertices(bool hasTexCoords = true, bool hasNormals = true, float minPos = -0.5f, float maxPos = 0.5f, float texCoordU = 1.0f, float texCoordV = 1.0f);

	std::vector<uint32_t> CreateCubeIndices();
	std::vector<uint32_t> CreatePlaneIndices();
}