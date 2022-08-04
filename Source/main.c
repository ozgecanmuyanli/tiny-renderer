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
#include "stb_image.h"

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
	for (unsigned int i = 0; i < RENDER_WIDTH * RENDER_HEIGHT; i++)
	{
		data[i * NUMBER_OF_CHANNELS] = red;
		data[i * NUMBER_OF_CHANNELS + 1] = green;
		data[i * NUMBER_OF_CHANNELS + 2] = blue;
	}
}

void clearDepthBuffer(float zValue, float* depthBuffer)
{
	for (unsigned int i = 0; i < RENDER_WIDTH * RENDER_HEIGHT; i++)
	{
		depthBuffer[i] = zValue;
	}
}

void setPixel(int red, int green, int blue, int x, int y, unsigned char* data)
{
	data[y * RENDER_WIDTH * NUMBER_OF_CHANNELS + x * NUMBER_OF_CHANNELS] = red;
	data[y * RENDER_WIDTH * NUMBER_OF_CHANNELS + x * NUMBER_OF_CHANNELS + 1] = green;
	data[y * RENDER_WIDTH * NUMBER_OF_CHANNELS + x * NUMBER_OF_CHANNELS + 2] = blue;
}

void setViewPort(PointF point, Point* screenPoint)
{
	screenPoint->x = (point.x + 1.0f) * RENDER_WIDTH / 2;
	screenPoint->y = (point.y + 1.0f) * RENDER_HEIGHT / 2;
}

int isInNDC(PointF point)
{
	if ((-1.0f <= point.x) && (-1.0f <= point.y) && (1.0f >= point.x) && (1.0f >= point.y))
		return 1;
	else
		return 0;
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

// gets barycentric texture coord and returns the color of the pixel 
// that corresponds to the texture coord
void getTextureColor(vec2 textCoord, int textureWidth, int textureHeight, int numOfChannels,
	unsigned char* texture, int* r, int* g, int* b)
{
	ivec2 texturePixels;
	texturePixels[0] = textCoord[0] * textureWidth;
	texturePixels[1] = fabs(textCoord[1] - 1.0f) * textureHeight;

	*r = texture[texturePixels[1] * textureWidth * numOfChannels + texturePixels[0] * numOfChannels];
	*g = texture[texturePixels[1] * textureWidth * numOfChannels + texturePixels[0] * numOfChannels + 1];
	*b = texture[texturePixels[1] * textureWidth * numOfChannels + texturePixels[0] * numOfChannels + 2];
}

// Function to check if three
// points make a triangle
bool checkTriangle(ivec2 p1, ivec2 p2, ivec2 p3)
{
	int a = p1[0] * (p2[1] - p3[1])
		+ p2[0] * (p3[1] - p1[1])
		+ p3[0] * (p1[1] - p2[1]);

	if (a == 0)
		return false;
	else
		return true;
}

// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
void Barycentric(ivec2 p, ivec2 a, ivec2 b, ivec2 c, vec3 barycentricCoords)
{
	bool isTriangle = checkTriangle(a, b, c);
	if (isTriangle)
	{
		double u, v, w;
		vec2 v0, v1, v2;
		v0[0] = b[0] - a[0];
		v0[1] = b[1] - a[1];
		v1[0] = c[0] - a[0];
		v1[1] = c[1] - a[1];
		v2[0] = p[0] - a[0];
		v2[1] = p[1] - a[1];
		double d00 = glm_vec2_dot(v0, v0);
		double d01 = glm_vec2_dot(v0, v1);
		double d11 = glm_vec2_dot(v1, v1);
		double d20 = glm_vec2_dot(v2, v0);
		double d21 = glm_vec2_dot(v2, v1);
		double denom = d00 * d11 - d01 * d01;
		v = (d11 * d20 - d01 * d21) / denom;
		w = (d00 * d21 - d01 * d20) / denom;
		u = 1.0f - v - w;
		barycentricCoords[0] = u;
		barycentricCoords[1] = v;
		barycentricCoords[2] = w;
	}
	else
	{
		barycentricCoords[0] = -1.0f;
		barycentricCoords[1] = -1.0f;
		barycentricCoords[2] = -1.0f;
	}
}

void drawTriangle(vec3 point1, vec3 point2, vec3 point3,
	vec2 textureCoord1, vec2 textureCoord2, vec2 textureCoord3, vec3 normal,
	int textureWidth, int textureHeight, int numOfChannels, unsigned char* texture,
	float* depthBuffer, unsigned char* data, int isFilled)
{
	PointF pointf1, pointf2, pointf3;
	pointf1.x = point1[0];
	pointf1.y = point1[1];
	pointf1.z = point1[2];
	pointf2.x = point2[0];
	pointf2.y = point2[1];
	pointf2.z = point2[2];
	pointf3.x = point3[0];
	pointf3.y = point3[1];
	pointf3.z = point3[2];
	if (isFilled)
	{
		vec3 bc_screen;
		ivec2 pixels;
		Point screenP1, screenP2, screenP3;
		ivec2 p1, p2, p3;
		vec2 bc_textureCoord;
		int r, g, b;
		float zValue;
		float intensity;

		setViewPort(pointf1, &screenP1);
		setViewPort(pointf2, &screenP2);
		setViewPort(pointf3, &screenP3);

		p1[0] = screenP1.x;
		p1[1] = screenP1.y;
		p2[0] = screenP2.x;
		p2[1] = screenP2.y;
		p3[0] = screenP3.x;
		p3[1] = screenP3.y;

		// for future light calculations
		vec3 light_dir = { 0.0f, 0.0f, 1.0f };
		glm_vec3_normalize(light_dir);
		glm_vec3_normalize(normal);
		intensity = glm_vec3_dot(normal, light_dir);

		/* get the bounding box of the triangle */
		int maxX = min(RENDER_WIDTH - 1, max(p1[0], max(p2[0], p3[0])));
		int minX = max(0, min(p1[0], min(p2[0], p3[0])));
		int maxY = min(RENDER_HEIGHT - 1, max(p1[1], max(p2[1], p3[1])));
		int minY = max(0, min(p1[1], min(p2[1], p3[1])));

		for (pixels[0] = minX; pixels[0] <= maxX; pixels[0]++)
		{
			for (pixels[1] = minY; pixels[1] <= maxY; pixels[1]++)
			{
				//if((pixels[0] == p1[0] && pixels[1] == p1[1]))
				//	Barycentric(pixels, p1, p2, p3, bc_screen);
				Barycentric(pixels, p1, p2, p3, bc_screen);

				if (bc_screen[0] < 0 || bc_screen[1] < 0 || bc_screen[2] < 0)
					continue;

				// texture sampler
				bc_textureCoord[0] = textureCoord1[0] * bc_screen[0] +
					textureCoord2[0] * bc_screen[1] +
					textureCoord3[0] * bc_screen[2];
				bc_textureCoord[1] = textureCoord1[1] * bc_screen[0] +
					textureCoord2[1] * bc_screen[1] +
					textureCoord3[1] * bc_screen[2];
				getTextureColor(bc_textureCoord, textureWidth, textureHeight, numOfChannels,
					texture, &r, &g, &b);

				// depth test
				zValue = point1[2] * bc_screen[0] + point2[2] * bc_screen[1] + point3[2] * bc_screen[2];
				if (zValue > depthBuffer[pixels[0] + pixels[1] * RENDER_WIDTH])
				{
					depthBuffer[pixels[0] + pixels[1] * RENDER_WIDTH] = zValue;
					setPixel(r, g, b, pixels[0], pixels[1], data);
				}
			}
		}
	}
	else
	{
		drawLine(255, 255, 255, pointf1, pointf2, data);
		drawLine(255, 255, 255, pointf2, pointf3, data);
		drawLine(255, 255, 255, pointf3, pointf1, data);
	}
}

void vertexShader(vec3 vertexPos, vec3 outputPos, mat4 model, mat4 view, mat4 projection)
{
	vec4 m, mv, mvp;
	float w;

	// mat4 * vec4 = vec4
	//local to world
	glm_mat4_mulv(model, (vec4) { vertexPos[0], vertexPos[1], vertexPos[2], 1.0f }, m);
	
	//world to eye
	glm_mat4_mulv(view, m, mv);

	//eye to clip
	glm_mat4_mulv(projection, mv, mvp);
	
	//clip to NDC
	// OpenGL uses w=-z because it converts the RH coordinate system to LF.
	// I only change the final Z position of the vertex to continue with the RH system.
	w = mvp[3]; // w=1 for orthographic projection
	outputPos[0] = mvp[0] / w;
	outputPos[1] = mvp[1] / w;
	outputPos[2] = -mvp[2] / w;
}

int main()
{
	OpenGLInit();

	unsigned char* data = 0;
	unsigned char* depthBuffer = 0;
	createColorBuffer(RENDER_WIDTH, RENDER_HEIGHT, &data);
	createDepthBuffer(RENDER_WIDTH, RENDER_HEIGHT, &depthBuffer);

	// obj load
	size_t numOfTriangles = LoadObjAndConvert("../../Resources/african_head.obj");
	if (0 == numOfTriangles)
	{
		printf("\nfailed to load & conv\n");
		return -1;
	}

	// texture load
	int textureWidth, textureHeight, numOfChannels, r, g, b;
	unsigned char* texture = stbi_load("../../Resources/african_head_diffuse.tga",
		&textureWidth, &textureHeight, &numOfChannels, 0);

	//transformation
	vec3 transformedP1, transformedP2, transformedP3;
	mat4 modelMatrix, viewMatrix, projectionMatrix;

	glm_mat4_identity(viewMatrix);
	glm_mat4_identity(projectionMatrix);
	glm_mat4_identity(modelMatrix);

	//glm_ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 10.0f, projectionMatrix);
	glm_perspective(glm_rad(45.0f), 1.0f, 0.1f, 100.0f, projectionMatrix);
	glm_lookat((vec3) { 0.0f, 0.0f, 3.0f }, (vec3) { 0.0f, 0.0f, 0.0f }, (vec3) { 0.0f, 1.0f, 0.0f }, viewMatrix);
	glm_scale(modelMatrix, (vec3) { 1.0f, 1.0f, 1.0f });

	while (!glfwWindowShouldClose(window))
	{
		clearColor(0, 0, 0, data);
		clearDepthBuffer(-1.0f, depthBuffer);
		glfwPollEvents();

		for (size_t i = 0; i < numOfTriangles; i++)
		{
			vertexShader(vertexArray[0 + i * 3], transformedP1, modelMatrix, viewMatrix, projectionMatrix);
			vertexShader(vertexArray[1 + i * 3], transformedP2, modelMatrix, viewMatrix, projectionMatrix);
			vertexShader(vertexArray[2 + i * 3], transformedP3, modelMatrix, viewMatrix, projectionMatrix);
			drawTriangle(
				transformedP1,
				transformedP2,
				transformedP3,
				textureArray[0 + i * 3],
				textureArray[1 + i * 3],
				textureArray[2 + i * 3],
				normalArray[i * 3],
				textureWidth,
				textureHeight,
				numOfChannels,
				texture,
				depthBuffer,
				data, 1
			);
		}
		textureData = data;
		MainLoop();
		glfwSwapBuffers(window);
		writeImage("../../../output_images/projection.png", RENDER_WIDTH, RENDER_HEIGHT,
			3, data, RENDER_WIDTH * NUMBER_OF_CHANNELS, 1);
	}
	glfwDestroyWindow(window);
	glfwTerminate();

	free(data);
	free(vertexArray);
	free(normalArray);
	free(textureArray);
	return 0;
}

