#pragma once
#include <string>
#include "Entity.h"
#include <DirectXMath.h>

namespace d3dcore
{
	struct TagComponent
	{
		std::string tag;

		TagComponent() = default;
		TagComponent(const TagComponent& rhs) = default;
		TagComponent(const std::string& tag) : tag(tag) {}
	};

	struct RelationshipComponent
	{
		Entity parent = { entt::null, nullptr };
		Entity first = { entt::null, nullptr }; // first child
		Entity next = { entt::null, nullptr }; // next sibling
		Entity prev = { entt::null, nullptr }; // prev sibling
	};

	struct TransformComponent
	{
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent& rhs) = default;
		TransformComponent(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale)
			: position(position), rotation(rotation), scale(scale)
		{
		}
		DirectX::XMMATRIX GetTransform()
		{
			using namespace DirectX;
			XMVECTOR quat = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
			return
				XMMatrixScaling(scale.x, scale.y, scale.z) *
				XMMatrixRotationQuaternion(quat) *
				XMMatrixTranslation(position.x, position.y, position.z);
		}

		DirectX::XMVECTOR GetForward()
		{
			return DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), GetTransform());
		}

		DirectX::XMVECTOR GetRight()
		{
			return DirectX::XMVector4Transform(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), GetTransform());
		}

		DirectX::XMVECTOR GetUp()
		{
			return DirectX::XMVector4Transform(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), GetTransform());
		}
	};
}