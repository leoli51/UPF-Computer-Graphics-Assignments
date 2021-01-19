#include "application.h"
#include "utils.h"
#include "image.h"

// *** global variables
// variable tells which assignment is drawing on the screen
int app_state = 1;

// task 1 variables 

// task 2 variables
int formula_shown = 0;

// task 3 variables 
Image normal_image;
Image grayscale_image;
Image channel_swap_image;
int filter_shown = 0;

// task 4 variables
int image_control = 0;
double angle = 0;
Image task4;
Image smalltask4;

// task 5 variables 
Image task5;
double anitime = 0; 

// task 6 variables
Image toolbar;
Image canvas;
Vector2 prev_mouse_pos;
Color draw_color(255,0,0);

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

	//loading mountains.tga
	normal_image.loadTGA("./mountain.tga");
	grayscale_image.loadTGA("./mountain.tga");
	channel_swap_image.loadTGA("./mountain.tga");
	normal_image.flipY();
	grayscale_image.flipY();
	channel_swap_image.flipY();
	grayscale_image.forEachPixel([](Color c){float rgb = (c.r + c.g + c.b) / 3.0f; return Color(rgb, rgb, rgb);});
	channel_swap_image.forEachPixel([](Color c){return Color(c.b, c.r, c.g);});

	//loading task4.tga
	task4.loadTGA("./task4.tga");
	task4.scale(window_width, window_height);
	task4.flipY();
	smalltask4.loadTGA("./task4.tga");
	smalltask4.scale(window_width / 5, window_height / 5);
	smalltask4.flipY();

	// task 5
	task5.resize(window_width*3, window_height*3);
	for (unsigned int x = 0; x < task5.width; x++){
		for (unsigned int y = 0; y < task5.height; y++){
			float plotdecide = randomValue();
			if (plotdecide <= 0.001){
				task5.setPixel(x, y, Color::WHITE);
			}
		}
	}

	// loading the toolbar 
	toolbar.loadTGA("./toolbar.tga");
	canvas.resize(window_width, window_height);
	canvas.fill(Color::WHITE);
	prev_mouse_pos.set(-1, -1);
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
		case 3: { // task 3
			if (filter_shown == 0){ // normal image
				framebuffer = normal_image;
			}
			else if (filter_shown == 1){ // grayscale
				framebuffer = grayscale_image;
			}
			else { // channel swap
				framebuffer = channel_swap_image;
			}
			break;
		}
		case 4: { // task4

			if (formula_shown == 0) //rotate image
			{
				if (image_control == 0) //couter-clockwize
				{
					for (unsigned int x = 0; x < smalltask4.width; x++)
					{
						for (unsigned int y = 0; y < smalltask4.height; y++)
						{
							int startx = window_width / 2;
							int starty = window_height / 2;
							int newx = x * cos(angle) - y * sin(angle) + startx;
							int newy = y * cos(angle) + x * sin(angle) + starty;
							framebuffer.setPixel(newx, newy, smalltask4.getPixel(x, y));
						}
					}
					
				}
				else // clockwize
				{
					for (unsigned int x = 0; x < smalltask4.width; x++)
					{
						for (unsigned int y = 0; y < smalltask4.height; y++)
						{
							int startx = window_width / 2;
							int starty = window_height / 2;
							int newx = x * cos(angle*-1) - y * sin(angle * -1) + startx;
							int newy = y * cos(angle * -1) + x * sin(angle * -1) + starty;
							framebuffer.setPixel(newx, newy, smalltask4.getPixel(x, y));
						}
					}
				}
			}
			else // scale image
			{
				if (image_control == 0) { // orginal size
					for (unsigned int x = 0; x < window_width / 1; x++) {
						for (unsigned int y = 0; y < window_height / 1; y++) {
							framebuffer.setPixel(x, y, task4.getPixel(x, y));
						}
					}
				}
				else if (image_control == 1) { //zoom out
					for (unsigned int x = 0; x < window_width; x++)
					{
						for (unsigned int y = 0; y < window_height; y++)
						{
							framebuffer.setPixel(x * 0.1 + window_width * 9 / 20, y * 0.1 + window_height * 9 / 20, task4.getPixel(x, y));
						}
					}
				}
				else //zoom in
				{
					for (unsigned int x = 0; x < window_width; x++)
					{
						for (unsigned int y = 0; y < window_height; y++)
						{
							framebuffer.setPixel(x, y, task4.getPixel(x * 0.1 + window_width * 0.4, y * 0.1 + window_height * 0.4));
						}
					}
				}
			}
			break;
		}
		case 5: { // task5
				for (int x = 0; x < window_width; x++)
				{
					for (int y = 0; y < window_height; y++)
					{
						if (y + anitime * 50 >= window_height * 3)
						{
							anitime = 0;
						}
						framebuffer.setPixel(x, y, task5.getPixelSafe(x + anitime * 50, y + anitime * 50));
					}
				}
		break;
		}
		case 6: { //task 6
			framebuffer = canvas;
			for (int xi = 0; xi < toolbar.width; xi++)
				for (int yi = 0; yi < toolbar.height; yi++)
					framebuffer.setPixelSafe(xi, framebuffer.height - yi, toolbar.getPixel(xi, yi));
			// and so on
		}
	}
}

//called after render
void Application::update(double seconds_elapsed){
	switch (app_state) {
		case 6: { // task 6
			if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)){
				if (mouse_position.y > window_height - toolbar.height){
					// check what icon has been clicked
					if (mouse_position.x < 50)
						canvas.fill(Color::WHITE);
					else if (mouse_position.x < 100)
						canvas.saveTGA("./BeatifulDrawing.tga");
					else if (mouse_position.x < 500){
						int cy = 25;
						int cx = snap(mouse_position.x - 25 , 50.0f) + 25;
						draw_color = toolbar.getPixel(cx, cy);
					}
					
				}
				else {
					//draw on canvas
					if (prev_mouse_pos.x != -1 && prev_mouse_pos.y != -1)
						canvas.drawLine(prev_mouse_pos.x, prev_mouse_pos.y, mouse_position.x, mouse_position.y, draw_color);
					prev_mouse_pos = mouse_position;
				}
			}
			else 
				prev_mouse_pos.set(-1, -1);

			break;
		}
		case 2: { // task 2 
			break;
		}
		case 4: { // task4
			angle = seconds_elapsed + angle;
			break;
		}
		case 5: { // task5
			anitime = seconds_elapsed + anitime;
			break;
		}
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
		filter_shown = 0;
		break;
	case SDL_SCANCODE_W:
		formula_shown = 1;
		filter_shown = 1;
		break;
	case SDL_SCANCODE_E:
		filter_shown = 2;
		break;
	case SDL_SCANCODE_Z:
		image_control = 0;
		break;
	case SDL_SCANCODE_X:
		image_control = 1;
		break;
	case SDL_SCANCODE_C:
		image_control = 2;
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


