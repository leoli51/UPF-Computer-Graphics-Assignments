#include "image.h"


Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width*height];
	memset(pixels, 0, width * height * sizeof(Color));
}

//copy constructor
Image::Image(const Image& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
}

//assign operator
Image& Image::operator = (const Image& c)
{
	if(pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height*sizeof(Color)];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
	return *this;
}

Image::~Image()
{
	if(pixels) 
		delete pixels;
}



//change image size (the old one will remain in the top-left corner)
void Image::resize(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for(unsigned int x = 0; x < min_width; ++x)
		for(unsigned int y = 0; y < min_height; ++y)
			new_pixels[ y * width + x ] = getPixel(x,y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

//change image size and scale the content
void Image::scale(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];

	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			new_pixels[ y * width + x ] = getPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)) );

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::getArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
{
	Image result(width, height);
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++x)
		{
			if( (x + start_x) < this->width && (y + start_y) < this->height) 
				result.setPixel( x, y, getPixel(x + start_x,y + start_y) );
		}
	return result;
}

void Image::flipX()
{
	for(unsigned int x = 0; x < width * 0.5; ++x)
		for(unsigned int y = 0; y < height; ++y)
		{
			Color temp = getPixel(width - x - 1, y);
			setPixel( width - x - 1, y, getPixel(x,y));
			setPixel( x, y, temp );
		}
}

void Image::flipY()
{
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height * 0.5; ++y)
		{
			Color temp = getPixel(x, height - y - 1);
			setPixel( x, height - y - 1, getPixel(x,y) );
			setPixel( x, y, temp );
		}
}


//Loads an image from a TGA file
bool Image::loadTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int bytesPerPixel;
	unsigned int imageSize;

	FILE * file = fopen(filename, "rb");
   	if ( file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
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
	if(pixels)
		delete pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width*height];

	//convert to float all pixels
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			this->setPixel(x , height - y - 1, Color( tgainfo->data[pos+2], tgainfo->data[pos+1], tgainfo->data[pos]) );
		}

	delete tgainfo->data;
	delete tgainfo;

	return true;
}

// Saves the image to a TGA file
bool Image::saveTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	FILE *file = fopen(filename, "wb");
	if ( file == NULL )
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
	unsigned char* bytes = new unsigned char[width*height*3];
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[(height-y-1)*width+x];
			unsigned int pos = (y*width+x)*3;
			bytes[pos+2] = c.r;
			bytes[pos+1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width*height*3, file);
	fclose(file);
	return true;
}

void Image::drawLineBresenham(int x0, int y0, int x1, int y1, const Color& c){
	// normalize coordinates (center line in 0,0)
	x1 -= x0;
	y1 -= y0;

	int dx = x1;
	int dy = y1;

	// compute octant
	int octant = 1;
	if (std::abs(dy) > dx) {
		// 2 / 3 / 6 / 7
		if (dy > 0) // 2 / 3
			octant = dx > 0 ? 2 : 3;
		else // 6 / 7
			octant = dx > 0 ?  7 : 6;
	}
	else {
		// 1 / 4 / 5 / 8
		if (dx > 0) // 1 / 8
			octant = dy > 0 ? 1 : 8;
		else // 4 / 5
			octant = dy > 0 ? 4 : 5;
	}

	// compute new x1, and y1 after transforms
	bresenhamLineTransform(&x1, &y1, octant);
	dx = x1;
	dy = y1;
	int x = 0;
	int y = 0;

	int diff_inc_E = 2*dy; // obtained by f(x+1, y) - f(x, y)
	int diff_inc_NE = 2*(dy - dx); // obtained by f(x+1, y+1) - f(x, y)
	int diff = 2*dy - dx; // 2 * difference of implicit function of line between start and midpoint

	bresenhamUndoLineTransform(&x, &y, octant); // return to original octant
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
		bresenhamUndoLineTransform(&x, &y, octant); // return to original octant 
		setPixelSafe(x + x0, y + y0, c); // paint pixel
		bresenhamLineTransform(&x, &y, octant); // return to first octant
	}
}

void Image::bresenhamLineTransform(int* x, int* y, int octant){
	int tmp = *x;
	switch (octant) {
		case 1:
			break;
		case 2:
			*x = *y;
			*y = tmp;
			break;
		case 3:
			*x = -*y;
			*y = tmp;
			break;
		case 4:
			*x *= -1;
			break;
		case 5:
			*x *= -1;
			*y *= -1;
			break;
		case 6:
			*x = -*y;
			*y = -tmp;
			break;
		case 7:
			*x = *y;
			*y = -tmp;
			break;
		case 8:
			*y *= -1;
			break;
	}
}

void Image::bresenhamUndoLineTransform(int* x, int* y, int octant){
	int tmp = *x;
	switch (octant) {
		case 1:
			break;
		case 2:
			*x = *y;
			*y = tmp;
			break;
		case 3:
			*x = *y;
			*y = -tmp;
			break;
		case 4:
			*x *= -1;
			break;
		case 5:
			*x *= -1;
			*y *= -1;
			break;
		case 6:
			*x = -*y;
			*y = -tmp;
			break;
		case 7:
			*x = -*y;
			*y = tmp;
			break;
		case 8:
			*y *= -1;
			break;
	}
}

void Image::drawLineFastBresenham( int x0 , int y0 , int x1 , int y1, const Color& color) {
	int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = ( dx > dy ? dx : -dy ) / 2 , e2 ;
	for ( ; ; ) {
		setPixelSafe(x0, y0, color);
		if (x0 == x1 && y0 == y1) break;
		e2 = err;
		if ( e2 > -dx ) { err -= dy ; x0 += sx ; }
		if ( e2 < dy ) { err += dx ; y0 += sy ; }
	}
}

void Image::drawLineDDA(int x1, int y1, int x2, int y2, const Color& color) {
	float x, y;
	float dx = (x2 - x1);
	float dy = (y2 - y1);
	float d = fabs(dx) >= fabs(dy) ? fabs(dx) : fabs(dy);
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

void Image::fillActiveEdgesTable(int x0, int y0, int x1, int y1, int min_y, int max_y, std::vector<Cells>& table) {
	float x = x0;
	float dy = (y1 - y0);

	if (dy != 0){
		float step_y = dy > 0? 1 : -1;
		float step_x = (x1 - x0) / std::abs(dy);
		for (int y = y0; y != y1; y+=step_y){
			x += step_x;
			int table_index = y - min_y;

			if (x < table[table_index].minx){
				table[table_index].minx = x;
			}
			if (x > table[table_index].maxx){
				table[table_index].maxx = x;
			}
		}
	}
	else { // handle horizontal line case
		int min_x = std::min(x0, x1);
		int max_x = std::max(x0, x1);
		int table_index = y0 - min_y;
		table[table_index].minx = std::min(min_x, table[table_index].minx);
		table[table_index].maxx = std::max(max_x, table[table_index].maxx);
	}
}


void Image::fillTriangleWithColor(Vector3 p0, Vector3 p1, Vector3 p2, const Color& color0, const Color& color1, const Color& color2, FloatImage& zbuffer) {
	int min_y = std::min(std::min(p0.y, p1.y), p2.y);
	int max_y = std::max(std::max(p0.y, p1.y), p2.y);

	int table_height = max_y - min_y + 1;

	std::vector<Cells> table(table_height);
	for (int i = 0; i < table.size(); i++) {
		table[i].minx = 100000; //very big number todo: change to std::numeric_limits<int>.max()...
		table[i].maxx = -100000; //very small number
	}

	// fill table
	fillActiveEdgesTable(p0.x, p0.y, p1.x, p1.y, min_y, max_y, table);
	fillActiveEdgesTable(p1.x, p1.y, p2.x, p2.y, min_y, max_y, table);
	fillActiveEdgesTable(p2.x, p2.y, p0.x, p0.y, min_y, max_y, table);

	Vector3 p;
	for (int py = min_y; py <= max_y; py++){
		for (int px = table[py - min_y].minx; px <= table[py - min_y].maxx; px++){
			//assuming p0,p1 and p2 are the vertices 2D
			p.set(px, py, 0);
			Vector3 bc = barycentricCoordinates(p, p0, p1, p2);
			p.z = p0.z * bc.x + p1.z * bc.y + p2.z * bc.z;

			// check if pixel is out of view
			if (p.z > 0 || p.x >= width || p.x < 0 || p.y >= height || p.y < 0)
				continue;
			// check if it occludes a pixel that is more in front
			if (p.z > zbuffer.getPixel(p.x, p.y)){
				zbuffer.setPixel(p.x, p.y, p.z);
				// use barycentric coordinates to compute color
				Color c = color0 * bc.x + color1 * bc.y + color2 * bc.z;
				// scale coords and set pixel
				setPixelSafe(p.x, p.y, c);
			}
		}
	}

}

void Image::fillTriangleWithTexture(Vector3 p0, Vector3 p1, Vector3 p2, Vector2 uv0, Vector2 uv1, Vector2 uv2, const Image& texture, FloatImage& zbuffer){
	int min_y = std::min(std::min(p0.y, p1.y), p2.y);
	int max_y = std::max(std::max(p0.y, p1.y), p2.y);

	int table_height = max_y - min_y + 1;

	std::vector<Cells> table(table_height);
	for (int i = 0; i < table.size(); i++) {
		table[i].minx = 100000; //very big number todo: change to std::numeric_limits<int>.max()...
		table[i].maxx = -100000; //very small number
	}
	
	// fill table
	fillActiveEdgesTable(p0.x, p0.y, p1.x, p1.y, min_y, max_y, table);
	fillActiveEdgesTable(p1.x, p1.y, p2.x, p2.y, min_y, max_y, table);
	fillActiveEdgesTable(p2.x, p2.y, p0.x, p0.y, min_y, max_y, table);

	Vector3 p;
	for (int py = min_y; py <= max_y; py++){
		for (int px = table[py - min_y].minx; px <= table[py - min_y].maxx; px++){
			//assuming p0,p1 and p2 are the vertices 2D
			p.set(px, py, 0);
			Vector3 bc = barycentricCoordinates(p, p0, p1, p2);
		
			//use weights to compute final uv
			Vector2 uv = uv0 * bc.x + uv1 * bc.y + uv2 * bc.z;

			// check if pixel uv is in coordinate range
			if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
				continue;
			// check if pixel is out of view
			if (p.z > 0 || p.x >= width || p.x < 0 || p.y >= height || p.y < 0)
				continue;
			// check if it occludes a pixel that is more in front
			if (p.z > zbuffer.getPixel(p.x, p.y)){
				zbuffer.setPixel(p.x, p.y, p.z);
				// scale coords and set pixel
				setPixelSafe(p.x, p.y, texture.getPixel(uv.x * texture.width, uv.y * texture.height));
			}
		}
	}

}


FloatImage::FloatImage(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new float[width*height];
	memset(pixels, 0, width * height * sizeof(float));
}

//copy constructor
FloatImage::FloatImage(const FloatImage& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width*height];
		memcpy(pixels, c.pixels, width*height * sizeof(float));
	}
}

//assign operator
FloatImage& FloatImage::operator = (const FloatImage& c)
{
	if (pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width*height * sizeof(float)];
		memcpy(pixels, c.pixels, width*height * sizeof(float));
	}
	return *this;
}

FloatImage::~FloatImage()
{
	if (pixels)
		delete pixels;
}


//change image size (the old one will remain in the top-left corner)
void FloatImage::resize(unsigned int width, unsigned int height)
{
	float* new_pixels = new float[width*height];
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


#ifndef IGNORE_LAMBDAS

//you can apply and algorithm for two images and store the result in the first one
//forEachPixel( img, img2, [](Color a, Color b) { return a + b; } );
template <typename F>
void forEachPixel(Image& img, const Image& img2, F f) {
	for(unsigned int pos = 0; pos < img.width * img.height; ++pos)
		img.pixels[pos] = f( img.pixels[pos], img2.pixels[pos] );
}

#endif