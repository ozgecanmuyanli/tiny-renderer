#include<stdio.h>
#include "stb_image_write.h"
#include "cglm/include/cglm/affine.h"

#define WIDTH 128
#define HEIGHT 128
#define NUMBER_OF_CHANNELS 3

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

void getCirclePoints(unsigned char* data)
{
	float angle = 0.0f;
	float centerX = (float)WIDTH / 2.0f;
	float centerY = (float)HEIGHT / 2.0f;
	float radius = (float)WIDTH / 4.0f;
	int x, y;
	for (size_t i = 0; i < 720; i++)
	{
		x = centerX + radius * cos(angle);
		y = centerY + radius * sin(angle);
		angle += 1.0f;
		setPixel(255, 0, 0, x, y, data);
	}
}

int main()
{
	unsigned char* data = 0;
	createBuffer(WIDTH, HEIGHT, &data);
	clearColor(102, 7, 185, data);
	
	getCirclePoints(data);
	writelmage("ozgis_output.png", WIDTH, HEIGHT, 3, data, WIDTH * NUMBER_OF_CHANNELS, true);
	printf("My first graphic app without OpenGL :) \n");

	return 0;
}
