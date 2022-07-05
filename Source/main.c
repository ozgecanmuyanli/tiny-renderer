#include <glad/glad.h>
#include <glfw-3.3.7/include/GLFW/glfw3.h>
#include <stdio.h>
#include "stb_image_write.h"
#include <stdlib.h>
#include "math.h"
#include "swap.h"
#include "loader.h"
#include "commonTypes.h"
#include "main.h"
#include "Window.h"

extern char* textureData;

void createColorBuffer(int width, int height, unsigned char** data)
{
	*data = calloc(NUMBER_OF_CHANNELS * width * height, sizeof(unsigned char));
}

void createDepthBuffer(int width, int height, float** depthBuffer)
{
	*depthBuffer = calloc(width * height, sizeof(float));
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

void clearDepthBuffer(float zValue, float* depthBuffer)
{
	for (unsigned int i = 0; i < WIDTH * HEIGHT; i++)
	{
		depthBuffer[i] = zValue;
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
	data[y * WIDTH * NUMBER_OF_CHANNELS + x * NUMBER_OF_CHANNELS] = red;
	data[y * WIDTH * NUMBER_OF_CHANNELS + x * NUMBER_OF_CHANNELS + 1] = green;
	data[y * WIDTH * NUMBER_OF_CHANNELS + x * NUMBER_OF_CHANNELS + 2] = blue;
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
	PointF point1, PointF point2, PointF point3, float* depthBuffer, unsigned char* data, int isFilled)
{
	if (isFilled)
	{
		if (isInNDC(point1) && isInNDC(point2) && isInNDC(point3))
		{
			vec3 bc_screen;
			ivec2 pixels;
			Point screenP1, screenP2, screenP3;
			float zValue;
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
			int maxX = min(WIDTH - 1, max(p1[0], max(p2[0], p3[0])));
			int minX = max(0, min(p1[0], min(p2[0], p3[0])));
			int maxY = min(HEIGHT - 1, max(p1[1], max(p2[1], p3[1])));
			int minY = max(0, min(p1[1], min(p2[1], p3[1])));

			for (pixels[0] = minX; pixels[0] <= maxX; pixels[0]++)
			{
				for (pixels[1] = minY; pixels[1] <= maxY; pixels[1]++)
				{
					Barycentric(pixels, p1, p2, p3, bc_screen);
					if (bc_screen[0] < 0 || bc_screen[1] < 0 || bc_screen[2] < 0)
						continue;
					zValue = point1.z * bc_screen[2] + point2.z * bc_screen[0] + point3.z * bc_screen[1];
					if (zValue > depthBuffer[pixels[0] + pixels[1] * WIDTH])
					{
						depthBuffer[pixels[0] + pixels[1] * WIDTH] = zValue;
						setPixel(red, green, blue, pixels[0], pixels[1], data);
					}
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
	OpenWindow(WIDTH / 2, HEIGHT / 2); //OpenGL window

	unsigned char* data = 0;
	unsigned char* depthBuffer = 0;
	createColorBuffer(WIDTH, HEIGHT, &data);
	createDepthBuffer(WIDTH, HEIGHT, &depthBuffer);

	vec3 light_dir = { 0.0f, 0.0f, 1.0f };
	float intensity0, intensity1, intensity2;
	size_t numOfTriangles = LoadObjAndConvert("../../Resources/african_head.obj");
	if (0 == numOfTriangles)
	{
		printf("\nfailed to load & conv\n");
		return -1;
	}

	while (1)
	{
		clearColor(0, 0, 0, data);
		clearDepthBuffer(-1.0f, depthBuffer);

		glm_vec3_normalize(light_dir);
		for (size_t i = 0; i < numOfTriangles; i++)
		{
			glm_vec3_normalize(normalArray[i * 3]);
			intensity0 = glm_vec3_dot(normalArray[i * 3], light_dir);

			drawTriangle(intensity0 * 255, intensity0 * 255, intensity0 * 255, vertexArray[0 + i * 3],
				vertexArray[1 + i * 3],
				vertexArray[2 + i * 3],
				depthBuffer,
				data, 1);
		}
		textureData = data;
		MainLoop();
	}

	writeImage("../../../output_images/light&depthTest.png", WIDTH, HEIGHT,
		3, data, WIDTH * NUMBER_OF_CHANNELS, 1);

	free(data);
	free(vertexArray);
	free(normalArray);
	return 0;
}

