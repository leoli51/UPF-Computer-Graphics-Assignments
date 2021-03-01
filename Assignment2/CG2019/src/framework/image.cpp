#include "image.h"

Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width * height];
	memset(pixels, 0, width * height * sizeof(Color));
}

//copy constructor
Image::Image(const Image& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new Color[width * height];
		memcpy(pixels, c.pixels, width * height * sizeof(Color));
	}
}

//assign operator
Image& Image::operator = (const Image& c)
{
	if (pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new Color[width * height * sizeof(Color)];
		memcpy(pixels, c.pixels, width * height * sizeof(Color));
	}
	return *this;
}

Image::~Image()
{
	if (pixels)
		delete pixels;
}

//change image size (the old one will remain in the top-left corner)
void Image::resize(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width * height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for (unsigned int x = 0; x < min_width; ++x)
		for (unsigned int y = 0; y < min_height; ++y)
			new_pixels[y * width + x] = getPixel(x, y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

//change image size and scale the content
void Image::scale(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width * height];

	for (unsigned int x = 0; x < width; ++x)
		for (unsigned int y = 0; y < height; ++y)
			new_pixels[y * width + x] = getPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)));

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::getArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
{
	Image result(width, height);
	for (unsigned int x = 0; x < width; ++x)
		for (unsigned int y = 0; y < height; ++y)
		{
			if ((x + start_x) < this->width && (y + start_y) < this->height)
				result.setPixel(x, y, getPixel(x + start_x, y + start_y));
		}
	return result;
}

void Image::flipX()
{
	for (unsigned int x = 0; x < width * 0.5; ++x)
		for (unsigned int y = 0; y < height; ++y)
		{
			Color temp = getPixel(width - x - 1, y);
			setPixel(width - x - 1, y, getPixel(x, y));
			setPixel(x, y, temp);
		}
}

void Image::flipY()
{
	for (unsigned int x = 0; x < width; ++x)
		for (unsigned int y = 0; y < height * 0.5; ++y)
		{
			Color temp = getPixel(x, height - y - 1);
			setPixel(x, height - y - 1, getPixel(x, y));
			setPixel(x, y, temp);
		}
}


//Loads an image from a TGA file
bool Image::loadTGA(const char* filename)
{
	unsigned char TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int bytesPerPixel;
	unsigned int imageSize;

	FILE* file = fopen(filename, "rb");
	if (file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
		fread(header, 1, sizeof(header), file) != sizeof(header))
	{
		std::cerr << "File not found: " << filename << std::endl;
		if (file == NULL)
			return NULL;
		else
		{
			fclose(file);
			return NULL;
		}
	}

	TGAInfo* tgainfo = new TGAInfo;

	tgainfo->width = header[1] * 256 + header[0];
	tgainfo->height = header[3] * 256 + header[2];

	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (header[4] != 24 && header[4] != 32))
	{
		std::cerr << "TGA file seems to have errors or it is compressed, only uncompressed TGAs supported" << std::endl;
		fclose(file);
		delete tgainfo;
		return NULL;
	}

	tgainfo->bpp = header[4];
	bytesPerPixel = tgainfo->bpp / 8;
	imageSize = tgainfo->width * tgainfo->height * bytesPerPixel;

	tgainfo->data = new unsigned char[imageSize];

	if (tgainfo->data == NULL || fread(tgainfo->data, 1, imageSize, file) != imageSize)
	{
		if (tgainfo->data != NULL)
			delete tgainfo->data;

		fclose(file);
		delete tgainfo;
		return false;
	}

	fclose(file);

	//save info in image
	if (pixels)
		delete pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width * height];

	//convert to float all pixels
	for (unsigned int y = 0; y < height; ++y)
		for (unsigned int x = 0; x < width; ++x)
		{
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			this->setPixel(x, height - y - 1, Color(tgainfo->data[pos + 2], tgainfo->data[pos + 1], tgainfo->data[pos]));
		}

	delete tgainfo->data;
	delete tgainfo;

	return true;
}

// Saves the image to a TGA file
bool Image::saveTGA(const char* filename)
{
	unsigned char TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	FILE* file = fopen(filename, "wb");
	if (file == NULL)
	{
		fclose(file);
		return false;
	}

	unsigned short header_short[3];
	header_short[0] = width;
	header_short[1] = height;
	unsigned char* header = (unsigned char*)header_short;
	header[4] = 24;
	header[5] = 0;

	//tgainfo->width = header[1] * 256 + header[0];
	//tgainfo->height = header[3] * 256 + header[2];

	fwrite(TGAheader, 1, sizeof(TGAheader), file);
	fwrite(header, 1, 6, file);

	//convert pixels to unsigned char
	unsigned char* bytes = new unsigned char[width * height * 3];
	for (unsigned int y = 0; y < height; ++y)
		for (unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[(height - y - 1) * width + x];
			unsigned int pos = (y * width + x) * 3;
			bytes[pos + 2] = c.r;
			bytes[pos + 1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width * height * 3, file);
	fclose(file);
	return true;
}

#ifndef IGNORE_LAMBDAS

//you can apply and algorithm for two images and store the result in the first one
//forEachPixel( img, img2, [](Color a, Color b) { return a + b; } );
template <typename F>
void forEachPixel(Image& img, const Image& img2, F f) {
	for (unsigned int pos = 0; pos < img.width * img.height; ++pos)
		img.pixels[pos] = f(img.pixels[pos], img2.pixels[pos]);
}

#endif

void Image::drawLineDDA(int x1, int y1, int x2, int y2, const Color& color) {
	float d;
	float x, y;
	float dx = (x2 - x1);
	float dy = (y2 - y1);
	if (fabs(dx) >= fabs(dy))
		d = fabs(dx);
	else
		d = fabs(dy);
	float vx = dx / d;
	float vy = dy / d;
	x = x1 + 0.5;
	y = y1 + 0.5;
	for (int i = 0; i <= d; i++)
	{
		setPixelSafe(x, y, color);
		x = x + vx;
		y = y + vy;
	}
}

void Image::drawLineDDA_table(int x0, int y0, int x1, int y1, std::vector<Cells>& table) {
	float d;
	float x, y;
	float dx = (x1 - x0);
	float dy = (y1 - y0);
	if (fabs(dx) >= fabs(dy))
		d = fabs(dx);
	else
		d = fabs(dy);
	float vx = dx / d;
	float vy = dy / d;
	x = x0;
	y = y0;
	for (int i = 0; i <= d; i++)
	{
		x = x + vx;
		y = y + vy;
		if (x < table[y].minx)
		{
			table[y].minx = x;
		}
		if (x > table[y].maxx)
		{
			table[y].maxx = x;
		}
	}
}

void Image::drawLineBresenham(int x0, int y0, int x1, int y1, const Color& c){
	// switch points if necessary (x1 < x0)
	if (x1 < x0){
		int tmp = x0;
		x0 = x1;
		x1 = tmp;
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	// normalize coordinates (center line in 0,0)
	x1 -= x0;
	y1 -= y0;

	int dx = x1;
	int dy = y1;

	// compute octant, rest of octants are not considered (handled by the point switching at the beginning)
	int octant = 1;
	if (std::abs(dy) > dx) 
		octant = dy > 0 ? 2 : 7;
	else 
		octant = dy < 0 ? 8 : 1;

	// compute new x1, and y1 after transforms
	bresenhamLineTransform(&x1, &y1, octant);
	dx = x1;
	dy = y1;
	int x = 0;
	int y = 0;

	int diff_inc_E = 2*dy; // obtained by f(x+1, y) - f(x, y)
	int diff_inc_NE = 2*(dy - dx); // obtained by f(x+1, y+1) - f(x, y)
	int diff = 2*dy - dx; // 2 * difference of implicit function of line between start and midpoint

	undoBresenhamLineTransform(&x, &y, octant); // return to original octant
	setPixelSafe(x + x0, y + y0, c); // paint pixel
	bresenhamLineTransform(&x, &y, octant); // return to first octant
	
	while (x < x1){
		if (diff <= 0){ // choose E
			diff += diff_inc_E; // update difference: add the difference of point E
			x++;
		}
		else { // choose NE
			diff += diff_inc_NE; // update difference: add the difference of point NE
			x++;
			y++;
		}
		undoBresenhamLineTransform(&x, &y, octant); // return to original octant 
		setPixelSafe(x + x0, y + y0, c); // paint pixel
		bresenhamLineTransform(&x, &y, octant); // return to first octant
	}
}

void Image::bresenhamLineTransform(int* x, int* y, int octant){
	if (octant == 2){
		// switch axes
		int tmp = *x;
		*x = *y;
		*y = tmp;
	}
	else if (octant == 8){
		// invert y
		*y *= -1;
	}
	else if (octant == 7){
		// multiply y by -1 and invert axes
		*y *= -1;
		int tmp = *x;
		*x = *y;
		*y = tmp;
	}
}

void Image::undoBresenhamLineTransform(int* x, int* y, int octant){
	if (octant == 2){
		// switch axes
		int tmp = *x;
		*x = *y;
		*y = tmp;
	}
	else if (octant == 8){
		// invert y
		*y *= -1;
	}
	else if (octant == 7){
		// invert axes and multiply y by -1
		int tmp = *x;
		*x = *y;
		*y = -tmp;
	}
}

void Image::drawCircleBresenham(int xc, int yc, int radius, const Color& c, bool fill){
	// first point is (r,0)
	// next we evaluate f(x, y) = x^2 + y^2 - r^2 on the midpoint: x -0.5 y + 1
	// if the f(x-0.5, y+1) >= 0 it means the midpoint is outside the circle so we set the (x-1, y+1) pixel
	// if the f(x-0.5, y+1) < 0 it means the midpoint is inside the circle so we set the (x, y+1) pixel
	// The pixel in the other octants are painted through reflection. 
	int x = radius;
	int y = 0;
	int diff = -4*x + 5; // 4 * diff = 8y - 4x + 5
	setPixelBresenhamCircle(x, y, xc, yc, c, fill);

	while (y < x){
		if (diff >= 0){
			// draw x-1, y+1
			diff += 8*y - 8*x + 4; 
			x--;
			y++;
		}
		else {
			// draw x, y+1
			diff += 8*y + 4;
			y++;
		}
		setPixelBresenhamCircle(x, y, xc, yc, c, fill);
	}
}

void Image::setPixelBresenhamCircle(int x, int y, int xc, int yc, const Color& c, bool fill){
	if (fill){
		for (int i = -x + xc; i <= x + xc; i++) // 1 - 4
			setPixelSafe(i, y + yc, c); 
		for (int i = -y + xc; i <= y + xc; i++) // 2 - 3
			setPixelSafe(i, x + yc, c);
		for (int i = -x + xc; i <= x + xc; i++) // 5 - 8
			setPixelSafe(i, -y + yc, c);
		for (int i = -y + xc; i <= y + xc; i++) // 6 - 7
			setPixelSafe(i, -x + yc, c);
	}
	else {
		setPixelSafe(x + xc, y + yc, c); // 1
		setPixelSafe(y + xc, x + yc, c); // 2
		setPixelSafe(-y + xc, x + yc, c); // 3
		setPixelSafe(-x + xc, y + yc, c); // 4
		setPixelSafe(-x + xc, -y + yc, c); // 5
		setPixelSafe(-y + xc, -x + yc, c); // 6
		setPixelSafe(y + xc, -x + yc, c); // 7
		setPixelSafe(x + xc, -y + yc, c); // 8
	}
}


void Image::drawtriangle(int x1, int y1, int x2, int y2, int x3, int y3, const Color& color, bool fill) {

	std::vector<Cells> table;
	table.resize(height);
	for (int i = 0; i < table.size(); i++) {
		table[i].minx = 100000; //very big number
		table[i].maxx = -100000; //very small number
	}
	if (fill)
	{
		drawLineDDA_table(x1, y1, x2, y2, table);
		drawLineDDA_table(x2, y2, x3, y3, table);
		drawLineDDA_table(x3, y3, x1, y1, table);

		for (int i = 0; i < table.size(); i++)
		{
			if (table[i].minx < table[i].maxx)
			{
				for (int start = table[i].minx; start <= table[i].maxx; start++)
				{
					setPixel(start, i, color);
				}
			}

		}
	}
	else
	{
		drawLineDDA(x1, y1, x2, y2, color);
		drawLineDDA(x2, y2, x3, y3, color);
		drawLineDDA(x3, y3, x1, y1, color);
	}

}

void Image::drawtriangle_interpolated(int x1, int y1, int x2, int y2, int x3, int y3, const Color& color1, const Color& color2, const Color& color3) {
	std::vector<Cells> table;
	table.resize(height);
	for (int i = 0; i < table.size(); i++) {
		table[i].minx = 100000; //very big number
		table[i].maxx = -100000; //very small number
	}
	drawLineDDA_table(x1, y1, x2, y2, table);
	drawLineDDA_table(x2, y2, x3, y3, table);
	drawLineDDA_table(x3, y3, x1, y1, table);

	for (int i = 0; i < table.size(); i++)
	{
		if (table[i].minx < table[i].maxx)
		{
			for (int start = table[i].minx; start <= table[i].maxx; start++)
			{
				Vector2 v0;
				v0.x = x3 - x1;
				v0.y = y3 - y1;
				Vector2 v1;
				v1.x = x2 - x1;
				v1.y = y2 - y1;
				Vector2 v2;
				v2.x = start - x1;
				v2.y = i - y1;


				float d00 = v0.dot(v0);
				float d01 = v0.dot(v1);
				float d11 = v1.dot(v1);
				float d20 = v2.dot(v0);
				float d21 = v2.dot(v1);
				float denom = d00 * d11 - d01 * d01;
				float v = (d11 * d20 - d01 * d21) / denom;
				float w = (d00 * d21 - d01 * d20) / denom;
				float u = 1.0 - v - w;

				// check if a point inside tirangle
				if (u>0 && v>0 && w>0)
				{
					Color c;
					c = color1 * u + color2 * v + color3 * w;
					setPixel(start, i, c);
				}

			}
		}

	}

}

//initialize table
//void inittable(int height, Cells) {
//
//}
