#ifndef COLOR3F_H
#define COLOR3F_H

#include <stdio.h>
#include <windows.h>

#include "RTWeekend.h"
#include "Vector3f.h"

void WriteAndOpenPPM(const Color3f* image, int width, int height)
{
	FILE* f = nullptr;

	fopen_s(&f, "Image.ppm", "w");

	if (!f)
		return;

	fprintf(f, "P3\n%d %d\n255\n", width, height);

	for (int i = 0; i < width * height; i++)
	{
		Color3f color = image[i];

		if (color.x != color.x) color.x = 0.0;
		if (color.y != color.y) color.y = 0.0;
		if (color.z != color.z) color.z = 0.0;

        color.x = powf(color.x, 1.0f / 2.2f);
        color.y = powf(color.y, 1.0f / 2.2f);
        color.z = powf(color.z, 1.0f / 2.2f);

		fprintf(f, "%d %d %d\n",
			static_cast<int>(256 * Clamp(color.x, 0.0f, 0.999f)),
			static_cast<int>(256 * Clamp(color.y, 0.0f, 0.999f)),
			static_cast<int>(256 * Clamp(color.z, 0.0f, 0.999f)));
	}

	fclose(f);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    ShellExecuteA(NULL, "open", "image.ppm", NULL, NULL, SW_SHOW);
}

#endif