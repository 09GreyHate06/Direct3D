#pragma once
#include <DirectXMath.h>

typedef int BOOL;

namespace cbufs
{
	namespace light
	{
		static constexpr uint32_t s_lightMaxCount = 32;

		struct VSSystemCBuf
		{
			struct
			{
				DirectX::XMFLOAT4X4 view;
				DirectX::XMFLOAT4X4 projection;
				DirectX::XMFLOAT3 viewPos;
				float p0;
			} camera;
		};

		struct VSEntityCBuf
		{
			struct
			{
				DirectX::XMFLOAT4X4 transformMatrix;
				DirectX::XMFLOAT4X4 normalMatrix;
			} entity;
		};

		struct PSSystemCbuf
		{
			struct
			{
				DirectX::XMFLOAT3 direction;
				float p0;
				DirectX::XMFLOAT3 ambient;
				float p1;
				DirectX::XMFLOAT3 diffuse;
				float p2;
				DirectX::XMFLOAT3 specular;
				float p3;
			} dirLights[s_lightMaxCount];

			struct
			{
				DirectX::XMFLOAT3 position;
				float p0;
				DirectX::XMFLOAT3 ambient;
				float constant;
				DirectX::XMFLOAT3 diffuse;
				float linear;
				DirectX::XMFLOAT3 specular;
				float quadratic;
			} pointLights[s_lightMaxCount];

			struct
			{
				DirectX::XMFLOAT3 position;
				float constant;

				DirectX::XMFLOAT3 direction;
				float linear;

				DirectX::XMFLOAT3 ambient;
				float quadratic;

				DirectX::XMFLOAT3 diffuse;
				float innerCutOffAngle;

				DirectX::XMFLOAT3 specular;
				float outerCutOffAngle;
			} spotLights[s_lightMaxCount];

			uint32_t activeDirLight;
			uint32_t activePointLight;
			uint32_t activeSpotLight;
			uint32_t p0;
		};

		struct PSEntityCBuf
		{
			struct
			{
				DirectX::XMFLOAT4 diffuseCol;
				DirectX::XMFLOAT2 tiling;
				float shininess;
				BOOL enableNormalMap;

			} material;
		};
	}


	namespace texture
	{
		struct VSSystemCBuf
		{
			DirectX::XMFLOAT4X4 view;
			DirectX::XMFLOAT4X4 projection;
		};

		struct VSEntityCBuf
		{
			DirectX::XMFLOAT4X4 transform;
		};

		struct PSEntityCBuf
		{
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT2 tiling;
			float p1;
			float p2;
		};
	}
}