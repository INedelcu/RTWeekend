#ifndef MATERIALS_H
#define MATERIALS_H

#include "Color3f.h"
#include "Geometry.h"
#include "Material.h"
#include "Sampling.h"
#include "Texture.h"
#include "Ray.h"
#include "RayPayload.h"

class Lambertian : public Material
{
public:
    Lambertian(const Color3f& c) : albedo(make_shared<SolidColorTexture>(c)) {}
    Lambertian(shared_ptr<Texture> tex) : albedo(tex) {}

    void ClosestHitShader(const Scene& scene, const RayDesc& rayDesc, const HitDesc& hitDesc, RayPayload& payload) const override
    {
        if (payload.rayDepth >= g_MaxRayDepthSolid)
        {
            payload.color = Color3f(0, 0, 0);
            return;
        }

        Vector3f hemDir = CosineWeightedSample(RandomFloat01(), RandomFloat01());

        Vector3f tangent, bitangent;
        FrisvadONB(hitDesc.normal, tangent, bitangent);

        RayDesc newRay;
        newRay.ray.direction = hemDir.x * tangent + hemDir.y * bitangent + hemDir.z * hitDesc.normal;
        newRay.ray.origin = hitDesc.position;
        newRay.tmin = g_TMin;
        newRay.tmax = g_TMax;

        RayPayload newRayPayload;
        newRayPayload.color = Color3f(1, 1, 1);
        newRayPayload.rayDepth = payload.rayDepth + 1;

        TraceRay(scene, newRay, newRayPayload);

        payload.color *= albedo->Sample(hitDesc.u, hitDesc.v) * newRayPayload.color;
    }

public:
    shared_ptr<Texture> albedo;
};

class LambertianWithCheckerTexture : public Material
{
public:
    LambertianWithCheckerTexture(shared_ptr<Texture> texOdd, shared_ptr<Texture> texEven) : albedoOdd(texOdd), albedoEven(texEven) {}

    void ClosestHitShader(const Scene& scene, const RayDesc& rayDesc, const HitDesc& hitDesc, RayPayload& payload) const override
    {
        if (payload.rayDepth >= g_MaxRayDepthSolid)
        {
            payload.color = Color3f(0, 0, 0);
            return;
        }

        Vector3f hemDir = CosineWeightedSample(RandomFloat01(), RandomFloat01());

        Vector3f tangent, bitangent;
        FrisvadONB(hitDesc.normal, tangent, bitangent);

        RayDesc newRay;
        newRay.ray.direction = hemDir.x * tangent + hemDir.y * bitangent + hemDir.z * hitDesc.normal;
        newRay.ray.origin = hitDesc.position;
        newRay.tmin = g_TMin;
        newRay.tmax = g_TMax;

        RayPayload newRayPayload;
        newRayPayload.color = Color3f(1, 1, 1);
        newRayPayload.rayDepth = payload.rayDepth + 1;

        TraceRay(scene, newRay, newRayPayload);

        float sines = sinf(10 * hitDesc.position.x) * sinf(10 * hitDesc.position.y) * sinf(10 * hitDesc.position.z);

        Color3f texColor = (sines < 0) ? albedoOdd->Sample(hitDesc.u, hitDesc.v) : albedoEven->Sample(hitDesc.u, hitDesc.v);

        payload.color *= texColor * newRayPayload.color;
    }

public:
    shared_ptr<Texture> albedoOdd;
    shared_ptr<Texture> albedoEven;
};

class Metal : public Material
{
public:
    Metal(const Color3f& albedo, float roughness)
    {
        this->albedo = albedo;
        this->roughness = std::max<float>(0.0f, std::min<float>(roughness, 0.99f));
    }

    void ClosestHitShader(const Scene& scene, const RayDesc& rayDesc, const HitDesc& hitDesc, RayPayload& payload) const override
    {
        if (payload.rayDepth >= g_MaxRayDepthSolid)
        {
            payload.color = Color3f(0, 0, 0);
            return;
        }

        RayDesc newRay;
        newRay.ray.direction = Normalize(Reflect(rayDesc.ray.direction, hitDesc.normal) + (roughness * RandomUnitVector()));
        newRay.ray.origin = hitDesc.position;
        newRay.tmin = g_TMin;
        newRay.tmax = g_TMax;

        RayPayload newRayPayload;
        newRayPayload.color = Color3f(1, 1, 1);
        newRayPayload.rayDepth = payload.rayDepth + 1;

        TraceRay(scene, newRay, newRayPayload);

        payload.color *= albedo * newRayPayload.color;
    }

public:
    Color3f albedo;
    float roughness;
};

class Dielectric : public Material
{
public:
    Dielectric(float ior)
    {
        this->ior = ior;

        // Assuming one of the mediums is air which has an index of refraction of ~1.
        this->invIor = 1.0f / ior;
    }

    static float Reflectance(float cosine, float iorRatio)
    {
        float r0 = (1 - iorRatio) / (1 + iorRatio);
        r0 = r0 * r0;
        float oneMinusCosine = 1.0f - cosine;
        float oneMinusCosine2 = oneMinusCosine * oneMinusCosine;
        return r0 + (1 - r0) * oneMinusCosine2 * oneMinusCosine2 * oneMinusCosine;
    }

    void ClosestHitShader(const Scene& scene, const RayDesc& rayDesc, const HitDesc& hitDesc, RayPayload& payload) const override
    {
        if (payload.rayDepth >= g_MaxRayDepthTransparent)
        {
            payload.color = Color3f(0, 0, 0);
            return;
        }

        RayDesc newRay;
        newRay.ray.origin = hitDesc.position;
        newRay.tmin = g_TMin;
        newRay.tmax = g_TMax;

        float iorRatio = hitDesc.frontFace ? invIor : ior;
        float cosTheta = -Dot(rayDesc.ray.direction, hitDesc.normal);
        float sinTheta = sqrtf(1 - cosTheta * cosTheta);
        bool tir = iorRatio * sinTheta > 1.0f;
        bool reflect = tir || Reflectance(cosTheta, iorRatio) > RandomFloat01();

        if (reflect)
        {
            // In case of total internal reflection, we just use the reflection vector.
            newRay.ray.direction = Reflect(rayDesc.ray.direction, hitDesc.normal);
        }
        else
        {
            newRay.ray.direction = Refract(rayDesc.ray.direction, hitDesc.normal, iorRatio);
        }

        RayPayload newRayPayload;
        newRayPayload.color = Color3f(1, 1, 1);
        newRayPayload.rayDepth = payload.rayDepth + 1;

        TraceRay(scene, newRay, newRayPayload);

        payload.color *= newRayPayload.color;
    }

public:
    float ior;
    float invIor;
};

#endif