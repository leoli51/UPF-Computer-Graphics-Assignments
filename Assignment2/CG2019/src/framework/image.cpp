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

		for (int i = 0; i <table.size(); i++)
		{
			if (table[i].minx < table[i].maxx)
			{
				drawLineDDA(table[i].minx, i, table[i].maxx, i, color);
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


//initialize table
//void inittable(int height, Cells) {
//
//}

#endif