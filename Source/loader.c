#include <memory.h>
#include <stdbool.h>
#include <string.h>

#include <float.h>
#include <limits.h>
#include <math.h>

#include "tinyobj_loader_c.h"

#ifdef _WIN64
#define atoll(S) _atoi64(S)
#include <windows.h>
#include<memoryapi.h>
#include "loader.h"
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

extern PointF* vertexArray;

typedef struct {
	unsigned int vb;
	int numTriangles;
} DrawObject;

static DrawObject gDrawObject;

static int width = 768;
static int height = 768;

static bool use_colors = false;
static bool draw_wireframe = true;

static const size_t OBJ_SIZE = sizeof(float) * 3 // pos
+ sizeof(float) * 3 // normal
+ sizeof(float) * 3 // color (based on normal)
+ sizeof(float) * 3; // color from material file.

static float prevMouseX, prevMouseY;
static int mouseLeftPressed;
static int mouseMiddlePressed;
static int mouseRightPressed;
static float curr_quat[4];
static float prev_quat[4];
static float eye[3], lookat[3], up[3];

static void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
	float v10[3];
	float v20[3];
	float len2;

	v10[0] = v1[0] - v0[0];
	v10[1] = v1[1] - v0[1];
	v10[2] = v1[2] - v0[2];

	v20[0] = v2[0] - v0[0];
	v20[1] = v2[1] - v0[1];
	v20[2] = v2[2] - v0[2];

	N[0] = v20[1] * v10[2] - v20[2] * v10[1];
	N[1] = v20[2] * v10[0] - v20[0] * v10[2];
	N[2] = v20[0] * v10[1] - v20[1] * v10[0];

	len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
	if (len2 > 0.0f) {
		float len = (float)sqrt((double)len2);

		N[0] /= len;
		N[1] /= len;
	}
}

static char* mmap_file(size_t* len, const char* filename) {
#ifdef _WIN64
	HANDLE file =
		CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (file == INVALID_HANDLE_VALUE) { /* E.g. Model may not have materials. */
		return NULL;
	}

	HANDLE fileMapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);

	LPVOID fileMapView = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
	char* fileMapViewChar = (char*)fileMapView;

	DWORD file_size = GetFileSize(file, NULL);
	(*len) = (size_t)file_size;

	return fileMapViewChar;
#else

	struct stat sb;
	char* p;
	int fd;

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		perror("open");
		return NULL;
	}

	if (fstat(fd, &sb) == -1) {
		perror("fstat");
		return NULL;
	}

	if (!S_ISREG(sb.st_mode)) {
		fprintf(stderr, "%s is not a file\n", filename);
		return NULL;
	}

	p = (char*)mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);

	if (p == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}

	if (close(fd) == -1) {
		perror("close");
		return NULL;
	}

	(*len) = sb.st_size;

	return p;

#endif
}

/* path will be modified */
static char* get_dirname(char* path) {
	char* last_delim = NULL;

	if (path == NULL) {
		return path;
	}

#if defined(_WIN32)
	/* TODO: Unix style path */
	last_delim = strrchr(path, '\\');
#else
	last_delim = strrchr(path, '/');
#endif

	if (last_delim == NULL) {
		/* no delimiter in the string. */
		return path;
	}

	/* remove '/' */
	last_delim[0] = '\0';

	return path;
}

static void get_file_data(void* ctx, const char* filename, const int is_mtl,
	const char* obj_filename, char** data, size_t* len) {
	// NOTE: If you allocate the buffer with malloc(),
	// You can define your own memory management struct and pass it through `ctx`
	// to store the pointer and free memories at clean up stage(when you quit an
	// app)
	// This example uses mmap(), so no free() required.
	(void)ctx;

	if (!filename) {
		//printf("null filename\n");
		(*data) = NULL;
		(*len) = 0;
		return;
	}

	size_t data_len = 0;

	*data = mmap_file(&data_len, filename);
	(*len) = data_len;
}

int LoadObjAndConvert(const char* filename) 
{
	tinyobj_attrib_t attrib;
	tinyobj_shape_t* shapes = NULL;
	size_t num_shapes;
	tinyobj_material_t* materials = NULL;
	size_t num_materials;
	PointF p1, p2, p3;

	{
		unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
		int ret =
			tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
				&num_materials, filename, get_file_data, NULL, flags);
		if (ret != TINYOBJ_SUCCESS) 
		{
			return 0;
		}

	}

	vertexArray = (PointF*)malloc(attrib.num_face_num_verts * 3 * sizeof(PointF));

	{
		DrawObject o;
		float* vb;
		/* std::vector<float> vb; //  */
		size_t face_offset = 0;
		size_t i;

		/* Assume triangulated face. */
		size_t num_triangles = attrib.num_face_num_verts;
		size_t stride =
			OBJ_SIZE /
			sizeof(float);

		vb = (float*)malloc(OBJ_SIZE * num_triangles * 3);

		for (i = 0; i < attrib.num_face_num_verts; i++) 
		{
			size_t f;
			for (f = 0; f < (size_t)attrib.face_num_verts[i] / 3; f++) 
			{
				size_t k;
				float v[3][3];
				float n[3][3];
				float c[3];
				float len2;

				tinyobj_vertex_index_t idx0 = attrib.faces[face_offset + 3 * f + 0];
				tinyobj_vertex_index_t idx1 = attrib.faces[face_offset + 3 * f + 1];
				tinyobj_vertex_index_t idx2 = attrib.faces[face_offset + 3 * f + 2];

				for (k = 0; k < 3; k++) 
				{
					int f0 = idx0.v_idx;
					int f1 = idx1.v_idx;
					int f2 = idx2.v_idx;

					v[0][k] = attrib.vertices[3 * (size_t)f0 + k];
					v[1][k] = attrib.vertices[3 * (size_t)f1 + k];
					v[2][k] = attrib.vertices[3 * (size_t)f2 + k];
				}
				p1.y = v[0][0];
				p1.x = v[0][1];
				p2.y = v[1][0];
				p2.x = v[1][1];
				p3.y = v[2][0];
				p3.x = v[2][1];

				vertexArray[0 + i * 3] = p1;
				vertexArray[1 + i * 3] = p2;
				vertexArray[2 + i * 3] = p3;

					
				if (attrib.num_normals > 0) 
				{
					int f0 = idx0.vn_idx;
					int f1 = idx1.vn_idx;
					int f2 = idx2.vn_idx;
					if (f0 >= 0 && f1 >= 0 && f2 >= 0) 
					{
						for (k = 0; k < 3; k++) 
						{
							n[0][k] = attrib.normals[3 * (size_t)f0 + k];
							n[1][k] = attrib.normals[3 * (size_t)f1 + k];
							n[2][k] = attrib.normals[3 * (size_t)f2 + k];
						}
					}
					else 
					{ /* normal index is not defined for this face */
				      /* compute geometric normal */
						CalcNormal(n[0], v[0], v[1], v[2]);
						n[1][0] = n[0][0];
						n[1][1] = n[0][1];
						n[1][2] = n[0][2];
						n[2][0] = n[0][0];
						n[2][1] = n[0][1];
						n[2][2] = n[0][2];
					}
				}
				else 
				{
					/* compute geometric normal */
					CalcNormal(n[0], v[0], v[1], v[2]);
					n[1][0] = n[0][0];
					n[1][1] = n[0][1];
					n[1][2] = n[0][2];
					n[2][0] = n[0][0];
					n[2][1] = n[0][1];
					n[2][2] = n[0][2];
				}

				for (k = 0; k < 3; k++) 
				{
					vb[(3 * i + k) * stride + 0] = v[k][0];
					vb[(3 * i + k) * stride + 1] = v[k][1];
					vb[(3 * i + k) * stride + 2] = v[k][2];
					vb[(3 * i + k) * stride + 3] = n[k][0];
					vb[(3 * i + k) * stride + 4] = n[k][1];
					vb[(3 * i + k) * stride + 5] = n[k][2];

					/* Set the normal as alternate color */
					c[0] = n[k][0];
					c[1] = n[k][1];
					c[2] = n[k][2];
					len2 = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
					if (len2 > 0.0f) 
					{
						float len = (float)sqrt((double)len2);

						c[0] /= len;
						c[1] /= len;
						c[2] /= len;
					}

					vb[(3 * i + k) * stride + 6] = (c[0] * 0.5f + 0.5f);
					vb[(3 * i + k) * stride + 7] = (c[1] * 0.5f + 0.5f);
					vb[(3 * i + k) * stride + 8] = (c[2] * 0.5f + 0.5f);

					/* now set the color from the material */
					if (attrib.material_ids[i] >= 0) 
					{
						int matidx = attrib.material_ids[i];
						vb[(3 * i + k) * stride + 9] = materials[matidx].diffuse[0];
						vb[(3 * i + k) * stride + 10] = materials[matidx].diffuse[1];
						vb[(3 * i + k) * stride + 11] = materials[matidx].diffuse[2];
					}
					else 
					{
						/* Just copy the default value */
						vb[(3 * i + k) * stride + 9] = vb[(3 * i + k) * stride + 6];
						vb[(3 * i + k) * stride + 10] = vb[(3 * i + k) * stride + 7];
						vb[(3 * i + k) * stride + 11] = vb[(3 * i + k) * stride + 8];
					}

				}
			}
			/* You can access per-face material through attrib.material_ids[i] */

			face_offset += (size_t)attrib.face_num_verts[i];
		}

		o.vb = 0;
		o.numTriangles = 0;
		if (num_triangles > 0) 
		{
			return num_triangles;
		}
		free(vb);

		gDrawObject = o;
	}

	tinyobj_attrib_free(&attrib);
	tinyobj_shapes_free(shapes, num_shapes);
	tinyobj_materials_free(materials, num_materials);

	return 1;
}