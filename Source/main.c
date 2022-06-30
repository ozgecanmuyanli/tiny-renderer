#include <glad/glad.h>
#include <glfw-3.3.7/include/GLFW/glfw3.h>
#include <stdio.h>
#include "stb_image_write.h"
#include <stdlib.h>
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

void setPixel(int red, int green, int blue, int x, int y, unsigned char* data)
{
	data[x * WIDTH * NUMBER_OF_CHANNELS + y * NUMBER_OF_CHANNELS] = red;
	data[x * WIDTH * NUMBER_OF_CHANNELS + y * NUMBER_OF_CHANNELS + 1] = green;
	data[x * WIDTH * NUMBER_OF_CHANNELS + y * NUMBER_OF_CHANNELS + 2] = blue;
}

void drawLine(int red, int green, int blue, PointF start, PointF end, unsigned char* data)
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

void drawTriangle(int red, int green, int blue,
	PointF point1, PointF point2, PointF point3, unsigned char* data)
{
	drawLine(red, green, blue, point1, point2, data);
	drawLine(red, green, blue, point2, point3, data);
	drawLine(red, green, blue, point3, point1, data);
}

void setViewPort(PointF point, Point* screenPoint)
{
	screenPoint->x = (point.x + 1.0f) * (float)WIDTH / 2.0f;
	screenPoint->y = (point.y + 1.0f) * (float)HEIGHT / 2.0f;
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
		drawTriangle(255, 255, 0, vertexArray[0 + i * 3], 
								  vertexArray[1 + i * 3], 
			                      vertexArray[2 + i * 3], 
			                      data);
	}

	writeImage("../../../output_images/african.png", WIDTH, HEIGHT, 
		       3, data, WIDTH * NUMBER_OF_CHANNELS, 1);

	textureData = data;
	OpenWindow(WIDTH/2, HEIGHT/2);
	MainLoop();

	free(data);
	free(vertexArray);
	return 0;
}
