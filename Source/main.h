#pragma once

char* textureData;

void createColorBuffer(int width, int height, unsigned char** data);
void createDepthBuffer(int width, int height, float** depthBuffer);
void clearDepthBuffer(float zValue, float* depthBuffer);
void writeImage(const char* filename, int width, int height, int comp,
	const void* data, int stride, unsigned int isFlipped);
void clearColor(int red, int green, int blue, unsigned char* data);
void setPixel(int red, int green, int blue, int x, int y, unsigned char* data);
void drawLine(int red, int green, int blue, PointF start, PointF end, unsigned char* data);
void drawTriangle(PointF point1, PointF point2, PointF point3,
	vec2 textureCoord1, vec2 textureCoord2, vec2 textureCoord3, vec3 normal,
	int textureWidth, int textureHeight, int numOfChannels, unsigned char* texture,
	float* depthBuffer, unsigned char* data, int isFilled);
void setViewPort(PointF point, Point* screenPoint);
int isInNDC(PointF point);
void Barycentric(ivec2 p, ivec2 a, ivec2 b, ivec2 c, vec3 barycentricCoords);
void getTextureColor(vec2 textCoord, int textureWidth, int textureHeight, int numOfChannels, 
	unsigned char* texture, int* r, int* g, int* b);
bool checkTriangle(ivec2 p1, ivec2 p2, ivec2 p3);