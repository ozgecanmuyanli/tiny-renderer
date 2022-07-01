#include <glad/glad.h>
#include <glfw-3.3.7/include/GLFW/glfw3.h>
#include <cglm/include/cglm/vec2.h>
#include <cglm/include/cglm/vec3.h>
#include <stdio.h>
#include "stb_image_write.h"
#include <stdlib.h>
#include "math.h"
#include "swap.h"
#include "loader.h"
#include "common.h"
#include "main.h"
#include "Window.h"

extern char* textureData;

void createBuffer(int width, int height, unsigned char** data)
{
	*data = calloc(NUMBER_OF_CHANNELS * width * height, sizeof(unsigned char));
}

void writeImage(const char* filename, int width, int height, int comp,
	const void* data, int stride, unsigned int isFlipped)
{
	stbi_flip_vertically_on_write(isFlipped);
	stbi_write_png(filename, width, height, comp, data, stride);
}

void clearColor(int red, int green, int blue, unsigned char* data)
{
	for (unsigned int i = 0; i < WIDTH * HEIGHT; i++)
	{
		data[i * NUMBER_OF_CHANNELS] = red;
		data[i * NUMBER_OF_CHANNELS + 1] = green;
		data[i * NUMBER_OF_CHANNELS + 2] = blue;
	}
}

void setViewPort(PointF point, Point* screenPoint)
{
	screenPoint->x = (point.x + 1.0f) * WIDTH / 2;
	screenPoint->y = (point.y + 1.0f) * HEIGHT / 2;
}

int isInNDC(PointF point)
{
	if ((-1.0f <= point.x) && (-1.0f <= point.y) && (1.0f >= point.x) && (1.0f >= point.y))
		return 1;
	else
		return 0;
}

// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
void Barycentric(ivec2 p, ivec2 a, ivec2 b, ivec2 c, vec3 barycentricCoords)
{
	float u, v, w;
    vec2 v0, v1, v2;
	v0[0] = b[0] - a[0];
	v0[1] = b[1] - a[1];
	v1[0] = c[0] - a[0];
	v1[1] = c[1] - a[1];
	v2[0] = p[0] - a[0];
	v2[1] = p[1] - a[1];
	float d00 = glm_vec2_dot(v0, v0);
	float d01 = glm_vec2_dot(v0, v1);
	float d11 = glm_vec2_dot(v1, v1);
	float d20 = glm_vec2_dot(v2, v0);
	float d21 = glm_vec2_dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;
	barycentricCoords[0] = u;
	barycentricCoords[1] = v;
	barycentricCoords[2] = w;
}

void setPixel(int red, int green, int blue, int x, int y, unsigned char* data)
{
	data[x * WIDTH * NUMBER_OF_CHANNELS + y * NUMBER_OF_CHANNELS] = red;
	data[x * WIDTH * NUMBER_OF_CHANNELS + y * NUMBER_OF_CHANNELS + 1] = green;
	data[x * WIDTH * NUMBER_OF_CHANNELS + y * NUMBER_OF_CHANNELS + 2] = blue;
}

void drawLine(int red, int green, int blue, PointF start, PointF end, unsigned char* data)
{
	if (isInNDC(start) && isInNDC(end))
	{
		Point startInPixel, endInPixel;
		setViewPort(start, &startInPixel);
		setViewPort(end, &endInPixel);

		unsigned int steep = 0;
		if (abs(startInPixel.x - endInPixel.x) < abs(startInPixel.y - endInPixel.y))
		{
			swap(&startInPixel.x, &startInPixel.y);
			swap(&endInPixel.x, &endInPixel.y);
			steep = 1;
		}
		if (startInPixel.x > endInPixel.x)
		{
			swap(&startInPixel.x, &endInPixel.x);
			swap(&startInPixel.y, &endInPixel.y);
		}
		int dx = endInPixel.x - startInPixel.x;
		int dy = endInPixel.y - startInPixel.y;
		int derror2 = abs(dy) * 2;
		int error2 = 0;
		int y = startInPixel.y;
		for (int x = startInPixel.x; x <= endInPixel.x; x++)
		{
			if (steep)
			{
				setPixel(red, green, blue, y, x, data);
			}
			else
			{
				setPixel(red, green, blue, x, y, data);
			}
			error2 += derror2;
			if (error2 > dx)
			{
				y += (endInPixel.y > startInPixel.y ? 1 : -1);
				error2 -= dx * 2;
			}
		}
	}
}

void drawTriangle(int red, int green, int blue,
	PointF point1, PointF point2, PointF point3, unsigned char* data, int isFilled)
{
	if (isFilled)
	{
		if (isInNDC(point1) && isInNDC(point2) && isInNDC(point3))
		{
			vec3 bc_screen;
			ivec2 pixels;
			Point screenP1, screenP2, screenP3;
			setViewPort(point1, &screenP1);
			setViewPort(point2, &screenP2);
			setViewPort(point3, &screenP3);

			ivec2 p1, p2, p3;
			p1[0] = screenP1.x;
			p1[1] = screenP1.y;
			p2[0] = screenP2.x;
			p2[1] = screenP2.y;
			p3[0] = screenP3.x;
			p3[1] = screenP3.y;

			/* get the bounding box of the triangle */
			int maxX = max(p1[0], max(p2[0], p3[0]));
			int minX = min(p1[0], min(p2[0], p3[0]));
			int maxY = max(p1[1], max(p2[1], p3[1]));
			int minY = min(p1[1], min(p2[1], p3[1]));
			for (pixels[0] = minX; pixels[0] <= maxX; pixels[0]++)
			{
				for (pixels[1] = minY; pixels[1] <= maxY; pixels[1]++)
				{
					Barycentric(pixels, p1, p2, p3, bc_screen);
					if (bc_screen[0] < 0 || bc_screen[1] < 0 || bc_screen[2] < 0) 
						continue;
					setPixel(red, green, blue, pixels[0], pixels[1], data);
				}
			}
		}
	}
	else
	{
		drawLine(red, green, blue, point1, point2, data);
		drawLine(red, green, blue, point2, point3, data);
		drawLine(red, green, blue, point3, point1, data);
	}
}

int main()
{
	unsigned char* data = 0;
	createBuffer(WIDTH, HEIGHT, &data);
	clearColor(0, 0, 0, data);

	int numOfTriangles = LoadObjAndConvert("../../Resources/african_head.obj");
	if (0 == numOfTriangles)
	{
		printf("\nfailed to load & conv\n");
		return -1;
	}
	for (size_t i = 0; i < numOfTriangles; i++)
	{
		int r = rand() % 255;
		int g = rand() % 255;
		int b = rand() % 255;
		drawTriangle(r, g, b, vertexArray[0 + i * 3],
			vertexArray[1 + i * 3],
			vertexArray[2 + i * 3],
			data, 1);
	}

	writeImage("../../../output_images/colored_filled_african_head.png", WIDTH, HEIGHT,
		3, data, WIDTH * NUMBER_OF_CHANNELS, 1);

	textureData = data;
	OpenWindow(WIDTH / 2, HEIGHT / 2);
	MainLoop();

	free(data);
	free(vertexArray);
	return 0;
}
