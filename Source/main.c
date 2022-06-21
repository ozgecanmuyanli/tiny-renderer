#include<stdio.h>
#include "stb_image_write.h"
#include "cglm/include/cglm/affine.h"

#define WIDTH 12
#define HEIGHT 12
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

int main()
{
	unsigned char* data = 0;
	createBuffer(WIDTH, HEIGHT, &data);
	clearColor(102, 7, 185, data);
	
	setPixel(255, 0, 0, 5, 5, data);
	writelmage("ozgis_output.png", WIDTH, HEIGHT, 3, data, WIDTH * NUMBER_OF_CHANNELS, true);
	printf("My first graphic app without OpenGL :) \n");

	return 0;
}
