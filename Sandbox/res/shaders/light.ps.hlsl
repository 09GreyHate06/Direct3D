struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 pixelPosition : PIXEL_POSITION;
    float3 viewPosition : VIEW_POSITION;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct Material
{
    float3 ambientCol;
    float p0;
    float3 diffuseCol;
    float p1;
    float3 specularCol;
    float p2;
    float2 tiling;
    float shininess;
    bool enableNormalMap;
};

struct DirectionalLight
{
    float3 direction;
    float p0;
    float3 ambient;
    float p1;
    float3 diffuse;
    float p2;
    float3 specular;
    float p3;
};

struct PointLight
{
    float3 position;
    float p0;
    float3 ambient;
    float attConstant;
    float3 diffuse;
    float attLinear;
    float3 specular;
    float attQuadratic;
};

struct SpotLight
{
    float3 position;
    float attConstant;
    float3 direction;
    float attLinear;
    
    float3 diffuse;
    float attQuadratic;
    float3 ambient;
    float innerCutOff; // cos
    float3 specular;
    float outerCutOff; // cos
};

static const uint s_lightMaxCount = 32;

cbuffer PSSystemCBuf : register(b0)
{
    DirectionalLight dirLights[s_lightMaxCount];
    PointLight pointLights[s_lightMaxCount];
    SpotLight spotLights[s_lightMaxCount];

    uint activeDirectionalLights = 0;
    uint activePointLights = 0;
    uint activeSpotLights = 0;
    uint p0;
}

cbuffer PSEntityCBuf : register(b1)
{
    Material material;
}

Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
Texture2D normalMap : register(t2);
SamplerState diffuseSampler : register(s0);
SamplerState specularSampler : register(s1);
SamplerState normalSampler : register(s2);

float3 Phong(float3 pixelToLight, float3 pixelToView, float3 normal, float3 lAmbient, float3 lDiffuse, float3 lSpecular, float3 mAmbient, float3 mDiffuse, float3 mSpecular);
float CalcAttenuation(float distance, float attConstant, float attLinear, float attQuadratic);

float4 main(VSOutput input) : SV_TARGET
{
    float3 pixelToView = normalize(input.viewPosition - input.pixelPosition);
    float3 normal = normalize(input.normal);
    if (material.enableNormalMap)
    {
        float3 t = normalize(input.tangent - dot(input.tangent, normal) * normal);
        float3 b = cross(normal, t);
        const float3x3 tanToWorld = float3x3(
            normalize(t),
            normalize(b),
            normalize(normal)
        );
        normal = normalMap.Sample(normalSampler, input.uv).xyz;
        normal = normal * 2.0f - 1.0f;
        normal = normalize(mul(normal, tanToWorld));
    }

    float3 mAmbient = (float3) diffuseMap.Sample(diffuseSampler, input.uv * material.tiling) * material.ambientCol;
    float3 mDiffuse = (float3) diffuseMap.Sample(diffuseSampler, input.uv * material.tiling) * material.diffuseCol;
    float3 mSpecular = (float3) specularMap.Sample(specularSampler, input.uv * material.tiling) * material.specularCol;

    float3 dirLightPhong = float3(0.0f, 0.0f, 0.0f);
    float3 pointLightPhong = float3(0.0f, 0.0f, 0.0f);
    float3 spotLightPhong = float3(0.0f, 0.0f, 0.0f);

    for (uint i = 0; i < activeDirectionalLights; i++)
    {
        DirectionalLight light = dirLights[i];
        float3 pixelToLight = normalize(-light.direction);
        dirLightPhong += Phong(pixelToLight, pixelToView, normal, light.ambient, light.diffuse, light.specular, mAmbient, mDiffuse, mSpecular);
    }

    for (uint i = 0; i < activePointLights; i++)
    {
        PointLight light = pointLights[i];
        float3 pixelToLight = normalize(light.position - input.pixelPosition);
        float attenuation = CalcAttenuation(length(light.position - input.pixelPosition), light.attConstant, light.attLinear, light.attQuadratic);
        float3 phong = Phong(pixelToLight, pixelToView, normal, light.ambient, light.diffuse, light.specular, mAmbient, mDiffuse, mSpecular);
        pointLightPhong += phong * attenuation;
    }

    for (uint i = 0; i < activeSpotLights; i++)
    {
        SpotLight light = spotLights[i];
        float3 pixelToLight = normalize(light.position - input.pixelPosition);
        float attenuation = CalcAttenuation(length(light.position - input.pixelPosition), light.attConstant, light.attLinear, light.attQuadratic);

        float theta = dot(pixelToLight, normalize(-light.direction));
        float epsilon = light.innerCutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);

        float3 phong = Phong(pixelToLight, pixelToView, normal, light.ambient, light.diffuse, light.specular, mAmbient, mDiffuse, mSpecular);
        spotLightPhong += phong * attenuation * intensity;
    }

    return float4(dirLightPhong + pointLightPhong + spotLightPhong, 1.0f);
}

float3 Phong(float3 pixelToLight, float3 pixelToView, float3 normal, float3 lAmbient, float3 lDiffuse, float3 lSpecular, float3 mAmbient, float3 mDiffuse, float3 mSpecular)
{
    float3 ambient = lAmbient * mAmbient;

    float diffuseFactor = max(dot(normal, pixelToLight), 0.0f);
    float3 diffuse = lDiffuse * mDiffuse * diffuseFactor;

    float3 halfwayDir = normalize(pixelToLight + pixelToView);
    float specularFactor = pow(max(dot(normal, halfwayDir), 0.0f), material.shininess);
    float3 specular = lSpecular * mSpecular * specularFactor;

    return ambient + diffuse + specular;
}

float CalcAttenuation(float distance, float attConstant, float attLinear, float attQuadratic)
{
    return 1.0f / (attConstant + attLinear * distance + attQuadratic * (distance * distance));

}