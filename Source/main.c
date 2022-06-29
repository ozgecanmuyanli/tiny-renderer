#include <stdio.h>
#include "stb_image_write.h"
#include <stdlib.h>
#include "loader.h"
#include "common.h"
#include "main.h"

void createBuffer(int width, int height, unsigned char** data)
{
	*data = calloc(NUMBER_OF_CHANNELS * width * height, sizeof(unsigned char));
}

void writelmage(const char* filename, int width, int height, int comp,
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
	PointF tmp;
	Point screenCoordInPixel;
	for (float increment = 0.0f; increment < 1.0f; increment += 0.001f)
	{
		tmp.x = start.x + (end.x - start.x) * increment;
		tmp.y = start.y + (end.y - start.y) * increment;
		setViewPort(tmp, &screenCoordInPixel);
		setPixel(red, green, blue, screenCoordInPixel.x, screenCoordInPixel.y, data);
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
	screenPoint->x = (point.x +1.0f)* (float)WIDTH / 2.0f;
	screenPoint->y = (point.y + 1.0f) * (float)HEIGHT / 2.0f;
}

int main()
{
	unsigned char* data = 0;
	createBuffer(WIDTH, HEIGHT, &data);
	clearColor(0, 0, 0, data);

	int numOfTriangles = LoadObjAndConvert("../Resources/african_head.obj");
	if (0 == numOfTriangles) 
	{
		printf("\nfailed to load & conv\n");
		return -1;
	}

	for (size_t i = 0; i < numOfTriangles; i++)
	{
		drawTriangle(255, 255, 0, vertexArray[0 + i * 3], vertexArray[1 + i * 3], vertexArray[2 + i * 3], data);
	}

	writelmage("african.png", WIDTH, HEIGHT, 3, data, WIDTH * NUMBER_OF_CHANNELS, 1);
	free(data);
	free(vertexArray);
	return 0;
}
