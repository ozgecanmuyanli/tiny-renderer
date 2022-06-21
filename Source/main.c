#include<stdio.h>
#include "stb_image_write.h"
#include "cglm/include/cglm/affine.h"

#define WIDTH 512
#define HEIGHT 512
#define NUMBER_OF_CHANNELS 3

typedef struct
{
	int x;
	int y;
}Point;

void createBuffer(int width, int height, unsigned char** data)
{
	*data = calloc(NUMBER_OF_CHANNELS * width * height, sizeof(unsigned char));
}

void writelmage(const char* filename, int width, int height, int comp,
	           const void* data, int stride, bool isFlipped)
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

void drawLine(int red, int green, int blue, Point start, Point end, unsigned char* data)
{
	int x, y;
	for (float increment = 0.0f; increment < 1.0f; increment += 0.001f)
	{
		x = start.x + (end.x - start.x) * increment;
		y = start.y + (end.y - start.y) * increment;
		setPixel(red, green, blue, x, y, data);
	}
}

void drawTriangle(int red, int green, int blue, 
	              Point point1, Point point2, Point point3, unsigned char* data)
{
	drawLine(red, green, blue, point1, point2, data);
	drawLine(red, green, blue, point2, point3, data);
	drawLine(red, green, blue, point3, point1, data);
}

int main()
{
	unsigned char* data = 0;
	createBuffer(WIDTH, HEIGHT, &data);
	clearColor(0, 0, 0, data);
	
	Point trianglePoint1 = { 200, 200 };
	Point trianglePoint2 = { 200, 300 };
	Point trianglePoint3 = { 300, 250 };
	drawTriangle(255, 255, 0, trianglePoint1, trianglePoint2, trianglePoint3, data);
	writelmage("triangle.png", WIDTH, HEIGHT, 3, data, 
		       WIDTH * NUMBER_OF_CHANNELS, true);
	printf("My first graphic app without OpenGL :) \n");

	return 0;
}
