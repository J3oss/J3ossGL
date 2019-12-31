#include "tgaimage.h"
#include "model.h"

const int width = 800;
const int height = 800;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

void cpu_optimized_triangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, TGAColor color)
{
	if (v0.y == v1.y && v0.y == v2.y) return;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	if (v0.y > v1.y) std::swap(v0, v1);
	if (v0.y > v2.y) std::swap(v0, v2);
	if (v1.y > v2.y) std::swap(v1, v2);

	int y = v0.y;

	int dx01 = v1.x - v0.x;
	int dy01 = v1.y - v0.y;
	int derror01 = std::abs(dx01) * 2;
	int error01 = 0;
	int xcounter01 = v0.x;
	int dy012 = dy01 * 2;

	int dx02 = v2.x - v0.x;
	int dy02 = v2.y - v0.y;
	int derror02 = std::abs(dx02) * 2;
	int error02 = 0;
	int xcounter02 = v0.x;
	int dy022 = dy02 * 2;

	int dx12 = v2.x - v1.x;
	int dy12 = v2.y - v1.y;
	int derror12 = std::abs(dx12) * 2;
	int error12 = 0;
	int xcounter12 = v1.x;
	int dy122 = dy12 * 2;

	for (; y <= v1.y; y++)
	{
		error01 += derror01;
		while (error01 > dy01 && dy01 !=0)
		{
			xcounter01 += (v1.x > v0.x ? 1 : -1);
			error01 -= dy012;
		}

		error02 += derror02;
		while (error02 > dy02 && dy02 != 0)
		{
			xcounter02 += (v2.x > v0.x ? 1 : -1);
			error02 -= dy022;
		}

		if (xcounter01 > xcounter02)
		{
			for (int j = xcounter02; j <= xcounter01; j++) {
				image.set(j, y, color);
			}
		}
		else
		{
			for (int j = xcounter01; j <= xcounter02; j++) {
				image.set(j, y, color);
			}
		}
	}

	for (; y <= v2.y; y++)
	{
		error12 += derror12;
		while (error12 > dy12 && dy12 != 0)
		{
			xcounter12 += (v2.x > v1.x ? 1 : -1);
			error12 -= dy122;
		}

		error02 += derror02;
		while (error02 > dy02 && dy02 != 0)
		{
			xcounter02 += (v2.x > v0.x ? 1 : -1);
			error02 -= dy022;
		}

		if (xcounter12 > xcounter02)
		{
			for (int j = xcounter02; j <= xcounter12; j++) {
				image.set(j, y, color);
			}
		}
		else
		{
			for (int j = xcounter12; j <= xcounter02; j++) {
				image.set(j, y, color);
			}
		}
	}
}

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);

	Model m = Model("obj/african_head.obj");
	Vec3f light_dir(0, 0, -1);

	for (int i = 0; i < m.nfaces(); i++)
	{
		std::vector<int> face = m.face(i);

		Vec3f world_co[3];
		Vec2i screen_co[3];
		
		for (int j = 0; j < 3; j++)
		{
			world_co[j] = m.vert(face[j]);
			screen_co[j] = Vec2i( (m.vert(face[j]).x + 1) * (width / 2) , (m.vert(face[j]).y + 1) * (height / 2) );
		}

		Vec3f normal = ((world_co[2] - world_co[0]) ^ (world_co[1] - world_co[0]));
		normal.normalize();

		float intensity = normal * light_dir;

		if (intensity > 0)
		{
			cpu_optimized_triangle(screen_co[0], screen_co[1], screen_co[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}