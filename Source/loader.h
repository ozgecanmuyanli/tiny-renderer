#pragma once
#include "commonTypes.h"

vec3* vertexArray;
vec3* normalArray;
vec2* textureArray;

int LoadObjAndConvert(const char* filename);
