#include "application.h"
#include "utils.h"
#include "image.h"
#include "mesh.h"

// camera position variables
float move_velocity = 50;
float mouse_sensitivity = 10;


Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);

	zbuffer.resize(w, h);
	framebuffer.resize(w, h);
}

//Here we have already GL working, so we can create meshes and textures
//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;
	
	//here we create a global camera and set a position and projection properties
	camera = new Camera();
	camera->lookAt(Vector3(0,10,-1000),Vector3(0,10,0),Vector3(0,1,0)); //define eye,center,up
	camera->perspective(60, window_width / (float)window_height, 0.1, 10000); //define fov,aspect,near,far

	//load a mesh
	mesh = new Mesh();
	if( !mesh->loadOBJ("lee.obj") )
		std::cout << "FILE Lee.obj NOT FOUND" << std::endl;

	//load the texture
	texture = new Image();
	texture->loadTGA("color.tga");
}

//this function fills the triangle by computing the bounding box of the triangle in screen space and using the barycentric interpolation
//to check which pixels are inside the triangle. It is slow for big triangles, but faster for small triangles /void Application::fillTriangle(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector2& uv0, const Vector2& uv1, const Vector2& uv2)
//{
	//compute triangle bounding box in screen space

	//clamp to screen area

	//loop all pixels inside bounding box
	// for()

		//we must compute the barycentrinc interpolation coefficients, weights of pixel P(x,y)

		//check if pixel is inside or outside the triangle

		//here add your code to test occlusions based on the Z of the vertices and the pixel (TASK 5)

		//here add your code to compute the color of the pixel (barycentric interpolation) (TASK 4)

		//draw the pixels in the colorbuffer x,y 
		//framebuffer.setPixel(x, y, COMPUTED COLOR);
//}

//render one frame
void Application::render(Image& framebuffer)
{
	framebuffer.fill(Color(40, 45, 60)); //clear

	//for every point of the mesh (to draw triangles take three points each time and connect the points between them (1,2,3,   4,5,6,   ...)
	for (int i = 0; i < mesh->vertices.size() / 100; i+=3)
	{
		Vector3 v0 = mesh->vertices[i]; //extract vertex from mesh
		Vector3 v1 = mesh->vertices[i + 1]; //extract vertex from mesh
		Vector3 v2 = mesh->vertices[i + 2]; //extract vertex from mesh

		//Vector2 texcoord = mesh->uvs[i]; //texture coordinate of the vertex (they are normalized, from 0,0 to 1,1)

		//project every point in the mesh to normalized coordinates using the viewprojection_matrix inside camera
		v0 = camera->projectVector(v0);
		v1 = camera->projectVector(v1);
		v2 = camera->projectVector(v2);

		//convert from normalized (-1 to +1) to framebuffer coordinates (0,W)
		v0.x = (v0.x + 1.0) / 2.0 * framebuffer.width;
		v1.x = (v1.x + 1.0) / 2.0 * framebuffer.width;
		v2.x = (v2.x + 1.0) / 2.0 * framebuffer.width;
		v0.y = (v0.y + 1.0) / 2.0 * framebuffer.height;
		v1.y = (v1.y + 1.0) / 2.0 * framebuffer.height;
		v2.y = (v2.y + 1.0) / 2.0 * framebuffer.height;

		//paint points in framebuffer (using your drawTriangle function or the fillTriangle function)
		framebuffer.drawtriangle(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, Color::WHITE, false);
	}
}

//called after render
void Application::update(double seconds_elapsed)
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	if (keystate[SDL_SCANCODE_SPACE])
	{
		//...
	}

	//example to move eye
	if (keystate[SDL_SCANCODE_LEFT])
		camera->eye.x -= move_velocity * seconds_elapsed;
	if (keystate[SDL_SCANCODE_RIGHT])
		camera->eye.x += move_velocity * seconds_elapsed;
	if (keystate[SDL_SCANCODE_DOWN])
		camera->eye.y -= move_velocity * seconds_elapsed;
	if (keystate[SDL_SCANCODE_UP])
		camera->eye.y += move_velocity * seconds_elapsed;

	camera->center.x += mouse_sensitivity * mouse_delta.x;
	camera->center.y += mouse_sensitivity * mouse_delta.y;

	//if we modify the camera fields, then update matrices
	camera->updateViewMatrix();
	camera->updateProjectionMatrix();
}

//keyboard press event 
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode)
	{
		case SDL_SCANCODE_ESCAPE: exit(0); break; //ESC key, kill the app
	}
}

//keyboard released event 
void Application::onKeyUp(SDL_KeyboardEvent event)
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode)
	{
	}
}

//mouse button event
void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{

	}
}

void Application::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse unpressed
	{

	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}
