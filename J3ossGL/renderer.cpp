#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <chrono>
#include "window_util.h"
#include <Windows.h>
#undef max
#undef min

const int depth = 255;

Model* m = NULL;
int* z_buffer = NULL;
int* pixels = NULL;


Vec3f camera(0, 0, 3);
Vec3f center(0, 0, 0);
Vec3f light_dir(0, 0, -1);
Vec3f up(0, 1, 0);

Matrix ViewPort(int x, int y, int w, int h) {
	Matrix m = Matrix::identity();
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}
Matrix ModelView(Vec3f camera, Vec3f center, Vec3f up) {
	Vec3f z = (camera - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	Matrix m_inv = Matrix::identity();
	Matrix tr = Matrix::identity();
	for (int i = 0; i < 3; i++) {
		m_inv[0][i] = x[i];
		m_inv[1][i] = y[i];
		m_inv[2][i] = z[i];
		tr[i][3] = -center[i];
	}
	return m_inv * tr;
}
Matrix V2M(Vec3f v) {
	Matrix m = Matrix::identity();
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}
Vec3f M2V(Matrix m) {
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Vec3f Barycentric(Vec3i *pts,Vec3f p)
{
	Vec3f vx = Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - p[0]);
	Vec3f vy = Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - p[1]);

	Vec3f bc = cross(vx,vy);

	if (std::abs(bc[2]) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (bc.x + bc.y) / bc.z, bc.y / bc.z, bc.x / bc.z);
}
void Triangle(Vec3i *pts,Vec2i *uv_pts,int *z_buffer,float intensity)
{
	Vec2i bbox_min = Vec2i(width - 1, height - 1);
	Vec2i bbox_max = Vec2i(0, 0);
	Vec2i clamp(width - 1, height - 1);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bbox_min[j] = std::max(0, std::min(bbox_min[j], pts[i][j]));
			bbox_max[j] = std::min(clamp[j], std::max(bbox_max[j], pts[i][j]));
		}
	}

	Vec3i p;
	for (p.x = bbox_min.x; p.x <= bbox_max.x; p.x++)
	{
		for (p.y = bbox_min.y; p.y <= bbox_max.y; p.y++)
		{
			Vec3f bc = Barycentric(pts, p);

			if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;

			p.z = bc * Vec3f(pts[0][2], pts[1][2], pts[2][2]);

			if (z_buffer[ (p.x + p.y* width) ] < p.z)
			{
				z_buffer[ (p.x + p.y * width) ] = p.z;

				int x = bc * Vec3f(uv_pts[0][0], uv_pts[1][0], uv_pts[2][0]);
				int y = bc * Vec3f(uv_pts[0][1], uv_pts[1][1], uv_pts[2][1]);

				auto color = m->diffuse(Vec2i(x,y));

				
				//image.set(p.x, p.y, TGAColor(intensity * color.r, intensity * color.g, intensity * color.b, 255));
				pixels[ (width*height) - (p.x + p.y * width)] = RGB(intensity * color.b, intensity * color.g, intensity * color.r);
			}		
		}
	}
}
Matrix z;
void init()
{
	m = new Model("obj/african_head/african_head.obj");

	Matrix model = Matrix::identity();	//can be used to setup the scene
	//model[2][3] = -5;

	Matrix model_view = ModelView(camera, center, up);

	Matrix projection = Matrix::identity();
	projection[3][2] = -1.f / camera.z - center.z;

	Matrix view_port = ViewPort(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	z = (view_port * projection * model_view * model);

	z_buffer = new int[width * height];
	for (int i = 0; i < width * height; i++)
	{
		z_buffer[i] = std::numeric_limits<int>::min();
	}

	pixels = new int[width * height];
	for (int i = 0; i < width * height; i++)
	{
		pixels[i] = 0;
	}
}

bool start = true;

int* render() 
{
	
	if (start)
	{
		init();
		start = false;
	}

	for (int i = 0; i < m->nfaces(); i++)
	{
		
		std::vector<int> face = m->face(i);

		Vec3f world_co[3];
		Vec3i screen_co[3];
		Vec2i text_co[3];

		for (int j = 0; j < 3; j++)
		{
			world_co[j] = m->vert(face[j]);
			screen_co[j] = M2V(z * V2M(world_co[j]));
		}


		Vec3f normal = cross(Vec3f(world_co[2] - world_co[0]) , Vec3f(world_co[1] - world_co[0]));

		normal.normalize();

		float intensity = normal * light_dir;

		if (intensity > 0)
		{
			for (int j = 0; j < 3; j++)
			{
				text_co[j] = m->uv(i, j);
			}
			Triangle(screen_co,text_co ,z_buffer, intensity);
		}
	}
	return pixels;
}