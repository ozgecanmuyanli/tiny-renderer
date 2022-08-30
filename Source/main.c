#include <glad/glad.h>
#include <glfw-3.3.7/include/GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include "stb_image_write.h"
#include "math.h"
#include "swap.h"
#include "loader.h"
#include "commonTypes.h"
#include "main.h"
#include "Window.h"
#include "stb_image.h"

//globals
extern char* textureData; // output image, to pass it to the openGL side as texture
int normalTextureWidth, normalTextureHeight, normalNumOfChannels;
int textureWidth, textureHeight, numOfChannels;
mat4 modelMatrix, viewMatrix, projectionMatrix;
mat3 TBN; //to change the spaces between world-tangent
vec3 lightDir, viewDir, normal;

enum triangleDrawingMode { FILLED = 1, MESH = 0 };


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

void setPixel(float red, float green, float blue, int x, int y, unsigned char* data)
{
	if (red < 0.0)
		red = 0;
	if (green < 0.0)
		green = 0;
	if (blue < 0.0)
		blue = 0;

	red *= 255;
	green *= 255;
	blue *= 255;

	data[y * RENDER_WIDTH * NUMBER_OF_CHANNELS + x * NUMBER_OF_CHANNELS] = red;
	data[y * RENDER_WIDTH * NUMBER_OF_CHANNELS + x * NUMBER_OF_CHANNELS + 1] = green;
	data[y * RENDER_WIDTH * NUMBER_OF_CHANNELS + x * NUMBER_OF_CHANNELS + 2] = blue;
}

void setViewPort(vec3 point, ivec2 screenPoint)
{
	screenPoint[0] = (point[0] + 1.0f) * RENDER_WIDTH / 2;
	screenPoint[1] = (point[1] + 1.0f) * RENDER_HEIGHT / 2;
}

int isInNDC(vec3 point)
{
	if ((-1.0f <= point[0]) && (-1.0f <= point[1]) && (1.0f >= point[0]) && (1.0f >= point[1]))
		return 1;
	else
		return 0;
}

void drawLine(int red, int green, int blue, vec3 start, vec3 end, unsigned char* data)
{
	if (isInNDC(start) && isInNDC(end))
	{
		ivec2 startInPixel, endInPixel;
		setViewPort(start, &startInPixel);
		setViewPort(end, &endInPixel);

		unsigned int steep = 0;
		if (abs(startInPixel[0] - endInPixel[0]) < abs(startInPixel[1] - endInPixel[1]))
		{
			swap(&startInPixel[0], &startInPixel[1]);
			swap(&endInPixel[0], &endInPixel[1]);
			steep = 1;
		}
		if (startInPixel[0] > endInPixel[0])
		{
			swap(&startInPixel[0], &endInPixel[0]);
			swap(&startInPixel[1], &endInPixel[1]);
		}
		int dx = endInPixel[0] - startInPixel[0];
		int dy = endInPixel[1] - startInPixel[1];
		int derror2 = abs(dy) * 2;
		int error2 = 0;
		int y = startInPixel[1];
		for (int x = startInPixel[0]; x <= endInPixel[0]; x++)
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
				y += (endInPixel[1] > startInPixel[1] ? 1 : -1);
				error2 -= dx * 2;
			}
		}
	}
}

// gets barycentric texture coord and returns the color of the pixel([0,1]) 
// that corresponds to the texture coord
void getTextureColor(vec2 textCoord, int textureWidth, int textureHeight, int numOfChannels,
	unsigned char* texture, float* r, float* g, float* b)
{
	ivec2 texturePixels;
	texturePixels[0] = textCoord[0] * textureWidth;
	texturePixels[1] = fabs(textCoord[1] - 1.0f) * textureHeight;

	*r = (float)(texture[texturePixels[1] * textureWidth * numOfChannels + texturePixels[0] * numOfChannels]) / 255.0f;
	*g = (float)(texture[texturePixels[1] * textureWidth * numOfChannels + texturePixels[0] * numOfChannels + 1]) / 255.0f;
	*b = (float)(texture[texturePixels[1] * textureWidth * numOfChannels + texturePixels[0] * numOfChannels + 2]) / 255.0f;
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

void drawTriangle(vertexBufferData triangleData,
	unsigned char* texture, unsigned char* textureNormal,
	float* depthBuffer, unsigned char* data, int mode)
{
	if (mode == FILLED)
	{
		vec3 bc_screen, bc_normalCoord, bc_vertexCoord, worldSpaceNormal;
		ivec2 screenP1, screenP2, screenP3, pixels;
		vec2 bc_textureCoord;
		float r, g, b, normalR, normalB, normalG;
		float zValue, intensity;
		vec3 lightDir = { 0.0, 0.0, 1.0f };
		mat3 inverseTBN;

		setViewPort(triangleData.vertexPos1, &screenP1);
		setViewPort(triangleData.vertexPos2, &screenP2);
		setViewPort(triangleData.vertexPos3, &screenP3);

		/* get the bounding box of the triangle */
		int maxX = min(RENDER_WIDTH - 1, max(screenP1[0], max(screenP2[0], screenP3[0])));
		int minX = max(0, min(screenP1[0], min(screenP2[0], screenP3[0])));
		int maxY = min(RENDER_HEIGHT - 1, max(screenP1[1], max(screenP2[1], screenP3[1])));
		int minY = max(0, min(screenP1[1], min(screenP2[1], screenP3[1])));

		for (pixels[0] = minX; pixels[0] <= maxX; pixels[0]++)
		{
			for (pixels[1] = minY; pixels[1] <= maxY; pixels[1]++)
			{
				Barycentric(pixels, screenP1, screenP2, screenP3, bc_screen);

				if (bc_screen[0] < 0 || bc_screen[1] < 0 || bc_screen[2] < 0)
					continue;

				// texture sampler
				bc_textureCoord[0] = triangleData.textureCoord1[0] * bc_screen[0] +
					triangleData.textureCoord2[0] * bc_screen[1] +
					triangleData.textureCoord3[0] * bc_screen[2];
				bc_textureCoord[1] = triangleData.textureCoord1[1] * bc_screen[0] +
					triangleData.textureCoord2[1] * bc_screen[1] +
					triangleData.textureCoord3[1] * bc_screen[2];

				//get color value of the pixel
				getTextureColor(bc_textureCoord, textureWidth, textureHeight, numOfChannels,
					texture, &r, &g, &b);
				getTextureColor(bc_textureCoord, normalTextureWidth, normalTextureHeight, normalNumOfChannels,
					textureNormal, &normalR, &normalG, &normalB);

				//interpolate the normal vectors
				bc_normalCoord[0] = triangleData.vertexNormal1[0] * bc_screen[0] +
					triangleData.vertexNormal2[0] * bc_screen[1] +
					triangleData.vertexNormal3[0] * bc_screen[2];
				bc_normalCoord[1] = triangleData.vertexNormal1[1] * bc_screen[0] +
					triangleData.vertexNormal2[1] * bc_screen[1] +
					triangleData.vertexNormal3[1] * bc_screen[2];
				bc_normalCoord[2] = triangleData.vertexNormal1[2] * bc_screen[0] +
					triangleData.vertexNormal2[2] * bc_screen[1] +
					triangleData.vertexNormal3[2] * bc_screen[2];

				calculateTBN(modelMatrix, triangleData.tangent, bc_normalCoord);

				normal[0] = (normalR) * 2.0 -1.0; //normalize normal vector to [-1, 1]
				normal[1] = (normalG) * 2.0 -1.0;
				normal[2] = (normalB) * 2.0 -1.0;
				
				//get world space normal by multiplying inverse TBN with tangent space normal
				glm_mat3_inv(TBN, inverseTBN);
				glm_mat3_mulv(inverseTBN, normal, worldSpaceNormal);
				
				//add light into the scene
				glm_normalize(lightDir);
				glm_normalize(worldSpaceNormal);
				intensity = glm_dot(worldSpaceNormal, lightDir);

				// depth test
				zValue = triangleData.vertexPos1[2] * bc_screen[0] + 
						 triangleData.vertexPos2[2] * bc_screen[1] + 
						 triangleData.vertexPos3[2] * bc_screen[2];
				if (zValue > depthBuffer[pixels[0] + pixels[1] * RENDER_WIDTH])
				{
					depthBuffer[pixels[0] + pixels[1] * RENDER_WIDTH] = zValue;
					setPixel(intensity * r, intensity * g, intensity * b, pixels[0], pixels[1], data);
				}
			}
		}
	}
	else if (mode == MESH)
	{
		drawLine(1.0f, 1.0f, 1.0f, triangleData.vertexPos1, triangleData.vertexPos2, data);
		drawLine(1.0f, 1.0f, 1.0f, triangleData.vertexPos2, triangleData.vertexPos3, data);
		drawLine(1.0f, 1.0f, 1.0f, triangleData.vertexPos3, triangleData.vertexPos1, data);
	}
}

//Calculates TBN matrix
void calculateTBN(mat4 model, vec3 tangent, vec3 normal)
{
	vec3 T, B, N, tmp;
	mat3 normalMatrix, mat3Model;

	mat3Model[0][0] = model[0][0];
	mat3Model[0][1] = model[0][1];
	mat3Model[0][2] = model[0][2];

	mat3Model[1][0] = model[1][0];
	mat3Model[1][1] = model[1][1];
	mat3Model[1][2] = model[1][2];

	mat3Model[2][0] = model[2][0];
	mat3Model[2][1] = model[2][1];
	mat3Model[2][2] = model[2][2];

	glm_mat3_inv(mat3Model, normalMatrix);
	glm_mat3_transpose(normalMatrix);
	glm_mat3_mulv(normalMatrix, tangent, T);
	glm_normalize(T);
	glm_mat3_mulv(normalMatrix, normal, N);
	glm_normalize(N);
	tmp[0] = glm_dot(T, N) * N[0];
	tmp[1] = glm_dot(T, N) * N[1];
	tmp[2] = glm_dot(T, N) * N[2];
	glm_vec3_sub(T, tmp, T);
	glm_normalize(T);
	glm_vec3_cross(N, T, B);

	TBN[0][0] = T[0];
	TBN[0][1] = T[1];
	TBN[0][2] = T[2];

	TBN[1][0] = B[0];
	TBN[1][1] = B[1];
	TBN[1][2] = B[2];

	TBN[2][0] = N[0];
	TBN[2][1] = N[1];
	TBN[2][2] = N[2];
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
	//OpenGL window to show the rendered image quickly
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
	unsigned char* texture = stbi_load("../../Resources/african_head_diffuse.tga",
		&textureWidth, &textureHeight, &numOfChannels, 0);

	unsigned char* textureNormal = stbi_load("../../Resources/african_head_nm_tangent.tga",
		&normalTextureWidth, &normalTextureHeight, &normalNumOfChannels, 0);

	//variables
	vec3 transformedP1, transformedP2, transformedP3;
	vertexBufferData triangleData;

	glm_mat4_identity(viewMatrix);
	glm_mat4_identity(projectionMatrix);
	glm_mat4_identity(modelMatrix);

	//glm_ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 10.0f, projectionMatrix);
	glm_perspective(glm_rad(45.0f), 1.0f, 0.1f, 100.0f, projectionMatrix);
	glm_lookat((vec3) { 0.0f, 0.0f, 3.0f }, (vec3) { 0.0f, 0.0f, 0.0f }, (vec3) { 0.0f, 1.0f, 0.0f }, viewMatrix);

	while (!glfwWindowShouldClose(window))
	{
		clearColor(0, 0, 0, data);
		clearDepthBuffer(-1.0f, depthBuffer);
		glfwPollEvents();

		//transformations
		glm_mat4_identity(modelMatrix);
		glm_rotate(modelMatrix, glfwGetTime(), (vec3) { 0.0, 1.0f, 0.0 });

		for (size_t i = 0; i < numOfTriangles; i++)
		{
			//calculate tangent value of a triangle
			//only one tangent vector is defined for a triangle(3-vertex)
			vec3 edge1, edge2, deltaUV1, deltaUV2, tangent;
			glm_vec3_sub(vertexArray[2 + i * 3], vertexArray[0 + i * 3], edge1);
			glm_vec3_sub(vertexArray[1 + i * 3], vertexArray[0 + i * 3], edge2);
			glm_vec3_sub(textureArray[2 + i * 3], textureArray[0 + i * 3], deltaUV1);
			glm_vec3_sub(textureArray[1 + i * 3], textureArray[0 + i * 3], deltaUV2);
			float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);
			tangent[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
			tangent[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
			tangent[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);

			//get projected output position from vertex shader
			vertexShader(vertexArray[0 + i * 3], transformedP1, modelMatrix, viewMatrix, projectionMatrix);
			vertexShader(vertexArray[1 + i * 3], transformedP2, modelMatrix, viewMatrix, projectionMatrix);
			vertexShader(vertexArray[2 + i * 3], transformedP3, modelMatrix, viewMatrix, projectionMatrix);

			//create buffer data
			memcpy(triangleData.vertexPos1, transformedP1, sizeof(transformedP1));
			memcpy(triangleData.vertexPos2, transformedP2, sizeof(transformedP2));
			memcpy(triangleData.vertexPos3, transformedP3, sizeof(transformedP3));
			memcpy(triangleData.textureCoord1, textureArray[0 + i * 3], sizeof(textureArray[0 + i * 3]));
			memcpy(triangleData.textureCoord2, textureArray[1 + i * 3], sizeof(textureArray[1 + i * 3]));
			memcpy(triangleData.textureCoord3, textureArray[2 + i * 3], sizeof(textureArray[2 + i * 3]));
			memcpy(triangleData.vertexNormal1, normalArray[0 + i * 3], sizeof(normalArray[0 + i * 3]));
			memcpy(triangleData.vertexNormal2, normalArray[1 + i * 3], sizeof(normalArray[1 + i * 3]));
			memcpy(triangleData.vertexNormal3, normalArray[2 + i * 3], sizeof(normalArray[2 + i * 3]));
			memcpy(triangleData.tangent, tangent, sizeof(tangent));

			drawTriangle(
				triangleData,
				texture,
				textureNormal,
				depthBuffer,
				data, 
				FILLED
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

