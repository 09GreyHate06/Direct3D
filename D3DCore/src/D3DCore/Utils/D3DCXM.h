#pragma once
#include <DirectXMath.h>

inline DirectX::XMFLOAT3 operator* (const DirectX::XMFLOAT3& v, const float s)
{
	return { v.x * s, v.y * s, v.z * s };
}

inline DirectX::XMFLOAT3 operator* (const float s, const DirectX::XMFLOAT3& v)
{
	return { v.x * s, v.y * s, v.z * s };
}

inline DirectX::XMFLOAT3 operator+ (const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline DirectX::XMFLOAT3 operator- (const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

namespace d3dcore::utils
{
    static DirectX::XMFLOAT3 ExtractEulerAngles(const DirectX::XMFLOAT4X4& transform)
    {
        DirectX::XMFLOAT3 euler;

        euler.x = asinf(-transform._32);                  // Pitch
       
        if (cosf(euler.x) > 0.0001)                // Not at poles
        {
            euler.y = atan2f(transform._31, transform._33);      // Yaw
            euler.z = atan2f(transform._12, transform._22);      // Roll
        }
        else
        {
            euler.y = 0.0f;                           // Yaw
            euler.z = atan2f(-transform._21, transform._11);     // Roll
        }

        //euler.x = atan2(transform._23, transform._33);
        //float cosYAngle = sqrt(pow(transform._11, 2) + pow(transform._12, 2));
        //euler.y = atan2(transform._13, cosYAngle);
        //float sinXAngle = sin(euler.x);
        //float cosXAngle = cos(euler.x);
        //euler.z = atan2(cosYAngle * transform._11 + sinXAngle * transform._21, cosXAngle * transform._12 + sinXAngle * transform._22);

        //const float PI = 3.14159265359f;
        //std::cout << euler.x * (180.0f / PI) << ", " << euler.y * (180.0f / PI) << ", " << euler.z * (180.0f / PI) << "\n";

        return euler;
    }

    static DirectX::XMFLOAT3 ExtractTranslation(const DirectX::XMFLOAT4X4& transform)
    {
        return { transform._41, transform._42, transform._43 };
    }

    static DirectX::XMFLOAT3 ExtractScale(const DirectX::XMFLOAT4X4& transform)
    {
        using namespace DirectX;
        XMVECTOR xmScaleX = XMVector3Length(XMVectorSet(transform._11, transform._12, transform._13, 0.0f));
        XMVECTOR xmScaleY = XMVector3Length(XMVectorSet(transform._21, transform._22, transform._23, 0.0f));
        XMVECTOR xmScaleZ = XMVector3Length(XMVectorSet(transform._31, transform._32, transform._33, 0.0f));

        float scaleX = XMVectorGetX(xmScaleX);
        float scaleY = XMVectorGetX(xmScaleY);
        float scaleZ = XMVectorGetX(xmScaleZ);

        return { scaleX, scaleY, scaleZ };
    }
}