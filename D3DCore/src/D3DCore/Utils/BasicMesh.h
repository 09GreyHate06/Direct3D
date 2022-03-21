#pragma once
#include <vector>
#include <array>
#include <DirectXMath.h>

namespace d3dcore::utils
{
	std::vector<float> CreateCubeVertices(bool hasTexCoords = true, bool hasNormals = true, float minPos = -0.5f, float maxPos = 0.5f, float texCoordU = 1.0f, float texCoordV = 1.0f);
	std::vector<float> CreatePlaneVertices(bool hasTexCoords = true, bool hasNormals = true, float minPos = -0.5f, float maxPos = 0.5f, float texCoordU = 1.0f, float texCoordV = 1.0f);

	std::array<uint32_t, 36> CreateCubeIndices();
	std::array<uint32_t, 6> CreatePlaneIndices();
}