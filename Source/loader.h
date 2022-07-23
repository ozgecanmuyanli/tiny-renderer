#pragma once
#include "commonTypes.h"

PointF* vertexArray;
vec3* normalArray;
vec2* textureArray;

int LoadObjAndConvert(const char* filename);
