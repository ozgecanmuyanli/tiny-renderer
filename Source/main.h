#pragma once

char* textureData;
typedef struct {
	vec3 vertexPos1;
	vec3 vertexPos2;
	vec3 vertexPos3;
	vec3 textureCoord1;
	vec3 textureCoord2;
	vec3 textureCoord3;
	vec3 vertexNormal1;
	vec3 vertexNormal2;
	vec3 vertexNormal3;
	vec3 tangent;
}vertexBufferData;

void createColorBuffer(int width, int height, unsigned char** data);
void createDepthBuffer(int width, int height, float** depthBuffer);
void clearDepthBuffer(float zValue, float* depthBuffer);
void writeImage(const char* filename, int width, int height, int comp,
	const void* data, int stride, unsigned int isFlipped);
void clearColor(int red, int green, int blue, unsigned char* data);
void setPixel(float red, float green, float blue, int x, int y, unsigned char* data);
void drawLine(int red, int green, int blue, vec3 start, vec3 end, unsigned char* data);
void drawTriangle(vertexBufferData triangleData,
	unsigned char* texture, unsigned char* textureNormal,
	float* depthBuffer, unsigned char* data, int mode);
void setViewPort(vec3 point, ivec2 screenPoint);
int isInNDC(vec3 point);
void Barycentric(ivec2 p, ivec2 a, ivec2 b, ivec2 c, vec3 barycentricCoords);
void getTextureColor(vec2 textCoord, int textureWidth, int textureHeight, int numOfChannels,
	unsigned char* texture, float* r, float* g, float* b);
bool checkTriangle(ivec2 p1, ivec2 p2, ivec2 p3);
void vertexShader(vec3 vertexPos, vec3 outputPos, mat4 model, mat4 view, mat4 projection);
void calculateTBN(mat4 model, vec3 tangent, vec3 normal);