#pragma once
#include <cglm/include/cglm/vec2.h>
#include <cglm/include/cglm/vec3.h>
#include <cglm/include/cglm/vec4.h>
#include <cglm/include/cglm/mat3.h>
#include <cglm/include/cglm/mat4.h>
#include <cglm/include/cglm/affine.h>
#include <cglm/include/cglm/cam.h>

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

#define WINDOW_WIDTH       (1024)
#define WINDOW_HEIGHT      (1024)
#define RENDER_WIDTH       (512)
#define RENDER_HEIGHT      (512)
#define NUMBER_OF_CHANNELS (3)

