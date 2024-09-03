#include <stdio.h>
#include <atomic>

#include "enkiTS/TaskScheduler.h"
#include "enkiTS/TaskScheduler_c.h"

#include "RTWeekend.h"
#include "Materials.h"
#include "Camera.h"
#include "Scene.h"
#include "Sphere.h"
#include "Texture.h"

Color3f* g_Output = nullptr;
uint32_t g_OutputWidth = 400;
uint32_t g_OutputHeight = 300;

Camera g_Camera;
Scene g_Scene;

thread_local uint64_t g_ThreadRayCount = 0;
std::atomic_uint64_t g_TotalRayCount = 0;

void MissShader(const RayDesc& rayDesc, RayPayload& payload);

// Inspired by https://learn.microsoft.com/en-us/windows/win32/direct3d12/traceray-function
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

void MissShader(const RayDesc& rayDesc, RayPayload& payload)
{
    // Background color.
    float t = 0.5f * (rayDesc.ray.direction.y + 1.0f);
    payload.color = ((1.0f - t) * Color3f(1.0f, 1.0f, 1.0f) + t * Color3f(0.5f, 0.7f, 1.0f));
}

void RayGenerationShader(uint32_t width, uint32_t height, uint32_t i, uint32_t j)
{
	Color3f pixelColor(0, 0, 0);

	const uint32_t samplesPerPixel = 1024;

	for (uint32_t s = 0; s < samplesPerPixel; s++)
	{
		float u = float(i + RandomFloat01()) / float(width);
		float v = float(j + RandomFloat01()) / float(height);

		Ray ray = g_Camera.GetRay(u, v);

		RayDesc rayDesc;
		rayDesc.ray = ray;
		rayDesc.tmin = g_TMin;
		rayDesc.tmax = g_TMax;

		RayPayload payload;
		payload.color = Color3f(1, 1, 1);
		payload.rayDepth = 0;

		TraceRay(g_Scene, rayDesc, payload);

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

// Scene 1: 3 Large Spheres + random smaller spheres using random materials.
void CreateScene1(Scene& scene)
{
    shared_ptr<Texture> checkerOdd = make_shared<SolidColorTexture>(Color3f(0.2f, 0.3f, 0.1f));
    shared_ptr<Texture> checkerEven = make_shared<SolidColorTexture>(Color3f(0.9f, 0.9f, 0.9f));

    shared_ptr<LambertianWithCheckerTexture> groundMaterial = make_shared<LambertianWithCheckerTexture>(checkerOdd, checkerEven);
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

                if (chooseMat < 0.6)
                {
                    // diffuse
                    Color3f albedo(RandomFloat01(), RandomFloat01(), RandomFloat01());
                    albedo *= albedo;
                    sphereMaterial = make_shared<Lambertian>(albedo);
                    scene.Add(make_shared<Sphere>(sphereMaterial, center, 0.2f));
                }
                else if (chooseMat < 0.7)
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
}

void CreateCamera(Camera& camera)
{
    float vFov = 20.0f;
    Vector3f lookFrom(13, 2, 3);
    Vector3f lookAt(0, 0, 0);
    float aspectRatio = float(g_OutputWidth) / float(g_OutputHeight);
    float aperture = 0.08f;
    float distToFocus = 10;

    camera = Camera(
        lookFrom,
        lookAt,
        Vector3f(0, 1, 0),
        vFov,
        aspectRatio,
        aperture,
        distToFocus);
}

void PathTraceScene()
{
    DispatchRaysData dispatchRaysData;
    dispatchRaysData.imageWidth = g_OutputWidth;
    dispatchRaysData.imageHeight = g_OutputHeight;

    enkiTaskScheduler* taskScheduler = enkiNewTaskScheduler();
    enkiInitTaskScheduler(taskScheduler);

    DisplayProgressJobData displayProgressJobData;
    displayProgressJobData.imageHeight = g_OutputHeight;

    enkiTaskSet* taskProgress = enkiCreateTaskSet(taskScheduler, DisplayProgressJob);
    enkiSetArgsTaskSet(taskProgress, &displayProgressJobData);
    enkiAddTaskSet(taskScheduler, taskProgress);

    enkiTaskSet* taskDispatchRays = enkiCreateTaskSet(taskScheduler, DispatchRaysJob);
    enkiAddTaskSetMinRange(taskScheduler, taskDispatchRays, &dispatchRaysData, g_OutputHeight, 1);

    enkiWaitForTaskSet(taskScheduler, taskDispatchRays);
    enkiWaitForTaskSet(taskScheduler, taskProgress);

    enkiDeleteTaskSet(taskScheduler, taskDispatchRays);
    enkiDeleteTaskSet(taskScheduler, taskProgress);

    enkiDeleteTaskScheduler(taskScheduler);
}

int main()
{	
	CreateScene1(g_Scene);

	CreateCamera(g_Camera);	

	g_Output = new Vector3f[g_OutputWidth * g_OutputHeight];

	printf("Generating output image.\n");

	clock_t t0 = clock();

	PathTraceScene();

	float deltaT = float(clock() - t0) / CLOCKS_PER_SEC;

	printf("\nDone!");

	printf("\nTime to generate image: %.2f seconds. Total rays: %I64u. Average path tracing speed: %.2f MRays/sec.", deltaT, g_TotalRayCount.load(), (double)g_TotalRayCount.load() / (deltaT * 1000000));

	printf("\nOpenning file...");

	WriteAndOpenPPM(g_Output, g_OutputWidth, g_OutputHeight);

	delete[] g_Output;

	return 0;
}

