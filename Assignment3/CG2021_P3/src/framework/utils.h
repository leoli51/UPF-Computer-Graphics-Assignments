/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This contains several functions that can be useful when programming your game.
*/
#ifndef UTILS_H
#define UTILS_H

#include "includes.h"
#include "framework.h"

//General functions **************
class Application;
class Image;

//check opengl errors
bool checkGLErrors();

SDL_Window* createWindow(const char* caption, int width, int height);
void launchLoop(Application* app);

void sendFramebufferToScreen(Image* img);

//fast random generator
inline unsigned long frand(void) {          //period 2^96-1
	unsigned long t;
	static unsigned long x = 123456789, y = 362436069, z = 521288629;
	x ^= x << 16; x ^= x >> 5; x ^= x << 1;
	t = x; x = y;y = z;
	z = t ^ x ^ y;
	return z;
}

inline float randomValue() { return (frand() % 10000) / 10000.0; }
std::string getBinPath();

// barycentric coordinates
inline Vector3 barycentricCoordinates(Vector3 p, Vector3 p0, Vector3 p1, Vector3 p2){
	Vector3 v0 = p1 - p0; 
	Vector3 v1 = p2 - p0;
	Vector3 v2 = p - p0; //p is the x,y of the pixel

	//computing the dot of a vector with itself
	//is the same as length*length but faster
	float d00 = v0.dot(v0);
	float d01 = v0.dot(v1);
	float d11 = v1.dot(v1);
	float d20 = v2.dot(v0);
	float d21 = v2.dot(v1);
	float denom = d00 * d11 - d01 * d01;
	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0 - v - w;

	return Vector3(u, v, w);
}

#endif
