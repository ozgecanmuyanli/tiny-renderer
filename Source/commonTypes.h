#pragma once
#include <cglm/include/cglm/vec2.h>
#include <cglm/include/cglm/vec3.h>
#include <cglm/include/cglm/mat3.h>

typedef struct
{
	int x;
	int y;
}Point;

typedef struct
{
	float x;
	float y;
	float z;
}PointF;

#define WIDTH (2048)
#define HEIGHT (2048)
#define NUMBER_OF_CHANNELS (3)
