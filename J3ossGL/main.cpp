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

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);

	Model m = Model("obj/african_head.obj");

	for (int i = 0; i < m.nfaces(); i++)
	{
		std::vector<int> face = m.face(i);

		for (int j = 0; j < 3; j++)
		{
			int x0 = (int)((m.vert(face[j]).x + 1) * (width / 2));
			int y0 = (int)((m.vert(face[j]).y + 1) * (height / 2));

			int x1 = (int)((m.vert(face[(j + 1) % 3]).x + 1) * (width / 2));
			int y1 = (int)((m.vert(face[(j + 1) % 3]).y + 1) * (height / 2));

			line(x0, y0, x1, y1, image, white);
		}
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}