#pragma once
#include "D3DCore/Utils/D3DCXM.h"
#include "D3DCore.h"
#include <stack>

static DirectX::XMMATRIX GetEntityParentsTransform(const d3dcore::RelationshipComponent& relationship)
{
	using namespace DirectX;
	using namespace d3dcore;

	XMMATRIX result = XMMatrixIdentity();

	if (Entity current = relationship.parent)
	{
		std::stack<XMMATRIX> parentsTransform;
		while (current)
		{
			parentsTransform.push(current.GetComponent<TransformComponent>().GetTransform());
			current = current.GetComponent<RelationshipComponent>().parent;
		}

		while (!parentsTransform.empty())
		{
			result *= parentsTransform.top();
			parentsTransform.pop();
		}
	}

	return result;
}