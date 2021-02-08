#include "application.h"
#include "utils.h"
#include "image.h"


// variable tells which task is drawing on the screen
int app_state = 1;
int formula_shown = 1;

// task 1 variables
int LineDDA_x1, LineDDA_x2;
int LineDDA_y1, LineDDA_y2;
int click = 1;

// task 2 variables
int bline_x0 = 0;
int bline_y0 = 0;
int bline_x1 = 0;
int bline_y1 = 0;

// task 3 variables
int bcircle_x = 0;
int bcircle_y = 0;
int bcircle_radius = 0;
bool bcircle_fill = true;

// task 4 variables
int triangle_x1, triangle_y1;
int triangle_x2, triangle_y2;
int triangle_x3, triangle_y3;
int triangle_click = 1;

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

	//here add your init stuff
}

//render one frame
void Application::render(Image& framebuffer)
{
	//clear framebuffer if we want to start from scratch
	framebuffer.fill(Color::BLACK);

	switch (app_state) {
		case 1: {
			framebuffer.drawLineDDA(LineDDA_x1, LineDDA_y1, LineDDA_x2, LineDDA_y2, Color::WHITE);
			break;
		}
		case 2: {
			framebuffer.drawLineBresenham(bline_x0, bline_y0, bline_x1, bline_y1, Color::CYAN);
			break;
		}
		case 3: {
			framebuffer.drawCircleBresenham(bcircle_x, bcircle_y, bcircle_radius, Color::WHITE, bcircle_fill);
			break;
		}
		case 4: {
			if (formula_shown == 1){
				// draw triangle if three points are determined
				if (triangle_click == 4){
					framebuffer.drawtriangle(triangle_x1, triangle_y1, triangle_x2, triangle_y2, triangle_x3, triangle_y3, Color::WHITE, 1);
				}
			}
			else{
				if (triangle_click == 4){
					framebuffer.drawtriangle_interpolated(triangle_x1, triangle_y1, triangle_x2, triangle_y2, triangle_x3, triangle_y3, Color::BLUE, Color::GREEN, Color::RED);
				}
			}
			break;
		}
	}
}

//called after render
void Application::update(double seconds_elapsed)
{
	switch (app_state) {
		case 2: {
			bline_x1 = mouse_position.x;
			bline_y1 = mouse_position.y;
			break;
		}
		case 3: {
			int tmp_x = mouse_position.x - bcircle_x;
			int tmp_y = mouse_position.y - bcircle_y;
			bcircle_radius = (int) std::sqrt(tmp_x * tmp_x + tmp_y * tmp_y);
			break;
		}
	}
}

//keyboard press event 
void Application::onKeyDown(SDL_KeyboardEvent event)
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode)
	{
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
	case SDL_SCANCODE_Q:
		formula_shown = 1;
		break;
	case SDL_SCANCODE_W:
		formula_shown = 2;
		break;
	case SDL_SCANCODE_F:
		bcircle_fill = !bcircle_fill;
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
	switch (app_state) {
	case 1: {
		if (event.button == SDL_BUTTON_LEFT) {
			if (click == 1)
			{
				LineDDA_x2 = mouse_position.x;
				LineDDA_y2 = mouse_position.y;
				LineDDA_x1 = mouse_position.x;
				LineDDA_y1 = mouse_position.y;
				click = 2;
			}
			else if (click == 2)
			{
				LineDDA_x2 = mouse_position.x;
				LineDDA_y2 = mouse_position.y;
				click = 1;
			}
		}
		break;
	}
	case 2: {
		if (event.button == SDL_BUTTON_LEFT){
			bline_x0 = mouse_position.x;
			bline_y0 = mouse_position.y;
		}
		break;
	}
	case 3: {
		if (event.button == SDL_BUTTON_LEFT){
			bcircle_x = mouse_position.x;
			bcircle_y = mouse_position.y;
		}
		break;
	}
	case 4: {
		if (event.button == SDL_BUTTON_LEFT) {
			if (triangle_click == 1)
			{
				triangle_x1 = mouse_position.x;
				triangle_y1 = mouse_position.y;
				triangle_click = 2;
			}
			else if (triangle_click == 2)
			{
				triangle_x2 = mouse_position.x;
				triangle_y2 = mouse_position.y;
				triangle_click = 3;
			}
			else if (triangle_click == 3)
			{
				triangle_x3 = mouse_position.x;
				triangle_y3 = mouse_position.y;
				triangle_click = 4;
			}
		}
		else if (event.button == SDL_BUTTON_RIGHT) //redraw triangle
		{
			triangle_click = 1;
		}
		break;
	}

	}

	//if you read mouse position from the event, careful, Y is reversed, use mouse_position instead

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
