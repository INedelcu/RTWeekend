#include <stdio.h>
#include <atomic>

#include "enkiTS/TaskScheduler.h"
#include "enkiTS/TaskScheduler_c.h"

#include "RTWeekend.h"
#include "Camera.h"
#include "Sampling.h"
#include "Scene.h"
#include "Sphere.h"
#include "AABB.h"

// A ray depth of 1 means that only primary rays can intersect geometries and evaluate their materials.
const int g_MaxRayDepthSolid = 6;
const int g_MaxRayDepthTransparent = 10;

const float g_TMin = 0.001f;
const float g_TMax = 100000.0f;

Color3f* g_Output = nullptr;

Camera camera;
Scene scene;

struct RayPayload
{
	Color3f		color;
	uint32_t	rayDepth;
};

void TraceRay(const Scene& scene, const RayDesc& rayDesc, RayPayload& payload);

void MissShader(const RayDesc& rayDesc, RayPayload& payload)
{
	// Background color.
	float t = 0.5f * (rayDesc.ray.direction.y + 1.0f);
	payload.color = ((1.0f - t) * Color3f(1.0f, 1.0f, 1.0f) + t * Color3f(0.5f, 0.7f, 1.0f));
}

class Lambertian : public Material
{
public:
	Lambertian(const Color3f& albedo)
	{
		this->albedo = albedo;
	}

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

		payload.color *= albedo * newRayPayload.color;
	}

public:
	Color3f albedo;
};

class Metal : public Material
{
public:
	Metal(const Color3f& albedo, float roughness)
	{
		this->albedo = albedo;
		this->roughness = std::max(0.0f, std::min(roughness, 0.99f));
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

thread_local uint64_t g_ThreadRayCount = 0;
std::atomic_uint64_t g_TotalRayCount = 0;

// Inspired from https://learn.microsoft.com/en-us/windows/win32/direct3d12/traceray-function
void TraceRay(const Scene& scene, const RayDesc& rayDesc, RayPayload& payload)
{
	g_ThreadRayCount++;

	HitDesc hitDesc;
	hitDesc.t = rayDesc.tmax;

	if (scene.Hit(rayDesc, hitDesc))
	{
		hitDesc.material->ClosestHitShader(scene, rayDesc, hitDesc, payload);
	}
	else
	{
		MissShader(rayDesc, payload);
	}
}

void RayGenerationShader(uint32_t width, uint32_t height, uint32_t i, uint32_t j)
{
	Color3f pixelColor(0, 0, 0);

	const uint32_t samplesPerPixel = 256;

	for (uint32_t s = 0; s < samplesPerPixel; s++)
	{
		float u = float(i + RandomFloat01()) / float(width);
		float v = float(j + RandomFloat01()) / float(height);

		Ray ray = camera.GetRay(u, v);

		RayDesc rayDesc;
		rayDesc.ray = ray;
		rayDesc.tmin = g_TMin;
		rayDesc.tmax = g_TMax;

		RayPayload payload;
		payload.color = Color3f(1, 1, 1);
		payload.rayDepth = 0;

		TraceRay(scene, rayDesc, payload);

		pixelColor += payload.color;
	}

	g_Output[width * (height - j - 1) + i] = pixelColor / samplesPerPixel;
}

struct DispatchRaysData
{
	uint32_t imageWidth;
	uint32_t imageHeight;
};

std::atomic_uint32_t g_ImageProgress = 0;

static void DispatchRaysJob(uint32_t start, uint32_t end, uint32_t threadnum, void* data)
{
	g_ThreadRayCount = 0;

	DispatchRaysData& dispatchRaysData = *(DispatchRaysData*)data;

	for (uint32_t y = start; y < end; y++)
	{
		for (uint32_t x = 0; x < dispatchRaysData.imageWidth; x++)
		{
			RayGenerationShader(dispatchRaysData.imageWidth, dispatchRaysData.imageHeight, x, y);
		}
	}

	g_TotalRayCount += g_ThreadRayCount;
	g_ImageProgress += end - start;
}

struct DisplayProgressJobData
{
	uint32_t imageHeight;
};

static void DisplayProgressJob(uint32_t start, uint32_t end, uint32_t threadnum, void* data)
{
	const DisplayProgressJobData& jobData = *(const DisplayProgressJobData*)data;

	while (g_ImageProgress.load() != jobData.imageHeight)
	{
		printf("\rPath tracing progress: %d%%", (int)(100 * g_ImageProgress.load() / (float)jobData.imageHeight));

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms);
	}

	printf("\rPath tracing progress: 100%%");
}

int main()
{
	const uint32_t imageWidth = 400;
	const uint32_t imageHeight = 300;
	
	shared_ptr<Lambertian> groundMaterial = make_shared<Lambertian>(Color3f(0.5f, 0.5f, 0.5f));
	scene.Add(make_shared<Sphere>(groundMaterial, Vector3f(0, -1000, 0), 1000.0f));
	
	for (int a = -11; a < 11; a++) 
	{
		for (int b = -11; b < 11; b++) 
		{
			float chooseMat = RandomFloat01();
			Vector3f center(a + 0.9f * RandomFloat01(), 0.2f, b + 0.9f * RandomFloat01());

			if ((center - Vector3f(4, 0.2f, 0)).Length() > 0.9)
			{
				shared_ptr<Material> sphereMaterial;

				if (chooseMat < 0.8)
				{
					// diffuse
					Color3f albedo(RandomFloat01(), RandomFloat01(), RandomFloat01());
					albedo *= albedo;
					sphereMaterial = make_shared<Lambertian>(albedo);
					scene.Add(make_shared<Sphere>(sphereMaterial, center, 0.2f));
				}
				else if (chooseMat < 0.95)
				{
					// metal
					Color3f albedo(RandomFloat01(), RandomFloat01(), RandomFloat01());
					float roughness = RandomFloat(0, 0.5);
					sphereMaterial = make_shared<Metal>(albedo, roughness);
					scene.Add(make_shared<Sphere>(sphereMaterial, center, 0.2f));
				}
				else 
				{
					// glass
					sphereMaterial = make_shared<Dielectric>(1.5f);
					scene.Add(make_shared<Sphere>(sphereMaterial, center, 0.2f));
				}
			}
		}
	}

	shared_ptr<Dielectric> material1 = make_shared<Dielectric>(1.5f);
	scene.Add(make_shared<Sphere>(material1, Vector3f(0, 1, 0), 1.0f));

	shared_ptr<Lambertian> material2 = make_shared<Lambertian>(Vector3f(0.4f, 0.2f, 0.1f));
	scene.Add(make_shared<Sphere>(material2, Vector3f(-4, 1, 0), 1.0f));

	shared_ptr<Metal> material3 = make_shared<Metal>(Color3f(0.7f, 0.6f, 0.5f), 0.0f);
	scene.Add(make_shared<Sphere>(material3, Vector3f(4, 1, 0), 1.0f));
	
	scene.BuildAccelerationStructure();

	float vFov = 20.0f;
	Vector3f lookFrom(13, 2, 3);
	Vector3f lookAt(0, 0, 0);
	float aspectRatio = float(imageWidth) / float(imageHeight);
	float aperture = 0.1f;
	float distToFocus = 10;

	camera = Camera(
		lookFrom,
		lookAt,
		Vector3f(0, 1, 0), 
		vFov, 
		aspectRatio,
		aperture,
		distToFocus);

	g_Output = new Vector3f[imageWidth * imageHeight];

	printf("Generating output image.\n");

	DispatchRaysData dispatchRaysData;
	dispatchRaysData.imageWidth = imageWidth;
	dispatchRaysData.imageHeight = imageHeight;

	clock_t t0 = clock();

	enkiTaskScheduler* taskScheduler = enkiNewTaskScheduler();
	enkiInitTaskScheduler(taskScheduler);

	DisplayProgressJobData displayProgressJobData;
	displayProgressJobData.imageHeight = imageHeight;

	enkiTaskSet* taskProgress = enkiCreateTaskSet(taskScheduler, DisplayProgressJob);
	enkiSetArgsTaskSet(taskProgress, &displayProgressJobData);
	enkiAddTaskSet(taskScheduler, taskProgress);

	enkiTaskSet* taskDispatchRays = enkiCreateTaskSet(taskScheduler, DispatchRaysJob);
	enkiAddTaskSetMinRange(taskScheduler, taskDispatchRays, &dispatchRaysData, imageHeight, 8);
	enkiWaitForTaskSet(taskScheduler, taskDispatchRays);

	enkiDeleteTaskSet(taskScheduler, taskDispatchRays);
	enkiDeleteTaskSet(taskScheduler, taskProgress);

	enkiDeleteTaskScheduler(taskScheduler);

	float deltaT = float(clock() - t0) / CLOCKS_PER_SEC;

	printf("\nTime to generate image: %.2f seconds. Total rays: %I64u. Average path tracing speed: %.2f MRays/sec.\n", deltaT, g_TotalRayCount.load(), (double)g_TotalRayCount.load() / (deltaT * 1000000));

	WritePPM(g_Output, imageWidth, imageHeight);

	delete[] g_Output;

	printf("Done\n");

	return 0;
}

