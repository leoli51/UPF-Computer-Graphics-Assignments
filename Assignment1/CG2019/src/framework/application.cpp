#include "application.h"
#include "utils.h"
#include "image.h"
#include<dos.h>  


// *** global variables
// variable tells which assignment is drawing on the screen
int app_state = 1;

// task 1 variables 

// task 2 variables
int formula_shown = 0;
int image_control = 0;
int angle = 0;


// task 3 variables 

// task 4 variables

// task 5 variables 

// task 6 variables

Image task4;
Image smalltask4;

Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);

	framebuffer.resize(w, h);
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;

	//loading task4.tga
	task4.loadTGA("../res/task4.tga");
	task4.scale(window_width, window_height);
	task4.flipY();
	smalltask4.loadTGA("../res/task4.tga");
	smalltask4.scale(window_width / 10, window_height / 10);
	smalltask4.flipY();

	//here add your init stuff
}

//render one frame
void Application::render(Image& framebuffer)
{
	//clear framebuffer if we want to start from scratch
	framebuffer.fill(Color::BLACK);

	switch (app_state) {
	case 1: { // task 1
		int w = framebuffer.width;
		int h = framebuffer.height;
		framebuffer.drawCircle(w / 10, h / 10, w / 10, Color::YELLOW);
		break;
	}
	case 2: { // task 2
		float radius = (framebuffer.height * framebuffer.height); //+ framebuffer.width * framebuffer.width); 
		for (int x = 0; x < framebuffer.width; x++)
			for (int y = 0; y < framebuffer.height; y++)
				if (formula_shown == 0) {
					int nx = x - framebuffer.width / 2;
					int ny = y - framebuffer.height / 2;
					Color c = Color::lerp(Color::BLACK, Color::WHITE, ((nx * nx) + (ny * ny)) / (radius));
					framebuffer.setPixel(x, y, c);
				}
				else {
					Color c = Color::lerp(Color::BLUE, Color::RED, ((float)x) / framebuffer.width);
					framebuffer.setPixel(x, y, c);
				}
		break;
	}
	case 4: { // task4

		if (formula_shown == 0) //rotate image
		{
			
			if (image_control == 0) //couter-clockwize
			{
				angle++;
				for (unsigned int x = 0; x < smalltask4.width; x++)
				{
					for (unsigned int y = 0; y < smalltask4.height; y++)
					{
						framebuffer.setPixel(x * cos(angle) - y * sin(angle) + window_width * 9 / 20, y * cos(angle) + x * sin(angle) + window_width * 9 / 20, smalltask4.getPixel(x, y));
					}
				}
				Sleep(300);
			}
			else // clockwize
			{
				angle--;
				for (unsigned int x = 0; x < smalltask4.width; x++)
				{
					for (unsigned int y = 0; y < smalltask4.height; y++)
					{
						framebuffer.setPixel(x * cos(angle) - y * sin(angle) + window_width * 9 / 20, y * cos(angle) + x * sin(angle) + window_width * 9 / 20, smalltask4.getPixel(x, y));
					}
				}
				Sleep(300);
			}
		}
		else // scale image
		{ 
			if (image_control == 0) //original size
			{
				framebuffer.fill(Color::BLACK);
				//task4.scale(window_width / 1, window_height / 1);

				for (unsigned int x = 0; x < window_width / 1; x++)
				{
					for (unsigned int y = 0; y < window_height / 1; y++)
					{
						framebuffer.setPixel(x, y, task4.getPixel(x, y));
					}
				}
			}
			else // 1/10 size
			{
				framebuffer.fill(Color::BLACK);
				for (unsigned int x = 0; x < window_width / 10; x++)
				{
					for (unsigned int y = 0; y < window_height / 10; y++)
					{
						framebuffer.setPixel(x + window_width * 9 / 20, y + window_height * 9 / 20, smalltask4.getPixel(x, y));
					}
				}
			}
		}
	}
		  // and so on
	}
}

//called after render
void Application::update(double seconds_elapsed)
{
	switch (app_state) {
	case 1: // task 1
		break;
	case 2: { // task 2 
	}
		  // and so on
	}
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	//if (keystate[SDL_SCANCODE_SPACE]) //if key space is pressed
	//to read mouse position use mouse_position
}

//keyboard press event 
void Application::onKeyDown(SDL_KeyboardEvent event)
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode)
	{
	case SDL_SCANCODE_ESCAPE:
		exit(0);
		break; //ESC key, kill the app
	case SDL_SCANCODE_1:
		app_state = 1;
		break;
	case SDL_SCANCODE_2:
		app_state = 2;
		break;
	case SDL_SCANCODE_3:
		app_state = 3;
		break;
	case SDL_SCANCODE_4:
		app_state = 4;
		break;
	case SDL_SCANCODE_5:
		app_state = 5;
		break;
	case SDL_SCANCODE_6:
		app_state = 6;
		break;
	case SDL_SCANCODE_Q:
		formula_shown = 0;
		break;
	case SDL_SCANCODE_W:
		formula_shown = 1;
		break;
	case SDL_SCANCODE_Z:
		image_control = 0;
		break;
	case SDL_SCANCODE_X:
		image_control = 1;
		break;
	}
}

//keyboard key up event 
void Application::onKeyUp(SDL_KeyboardEvent event)
{
	//...
}

//mouse button event
void Application::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{
		//if you read mouse position from the event, careful, Y is reversed, use mouse_position instead
	}
}

void Application::onMouseButtonUp(SDL_MouseButtonEvent event)
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


