#pragma once

void createBuffer(int width, int height, unsigned char** data);
void writelmage(const char* filename, int width, int height, int comp,
	const void* data, int stride, unsigned int isFlipped);
void clearColor(int red, int green, int blue, unsigned char* data);
void setPixel(int red, int green, int blue, int x, int y, unsigned char* data);
void drawLine(int red, int green, int blue, Point start, Point end, unsigned char* data);
void drawTriangle(int red, int green, int blue,
	Point point1, Point point2, Point point3, unsigned char* data);
