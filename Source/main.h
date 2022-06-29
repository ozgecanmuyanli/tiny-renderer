#pragma once

char* textureData;

void createBuffer(int width, int height, unsigned char** data);
void writelmage(const char* filename, int width, int height, int comp,
	const void* data, int stride, unsigned int isFlipped);
void clearColor(int red, int green, int blue, unsigned char* data);
void setPixel(int red, int green, int blue, int x, int y, unsigned char* data);
void drawLine(int red, int green, int blue, PointF start, PointF end, unsigned char* data);
void drawTriangle(int red, int green, int blue,
	PointF point1, PointF point2, PointF point3, unsigned char* data);
void setViewPort(PointF point, Point* screenPoint);
