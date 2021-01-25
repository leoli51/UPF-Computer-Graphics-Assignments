#include "application.h"
#include "utils.h"
#include "image.h"

// *** global variables
// variable tells which task is drawing on the screen
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
float rotation_dir = 1;
double angle = 0;
float angular_velocity = 2;
float scale = 1;
float scale_velocity = 2;
Image task4;
Image smalltask4;

// task 5 variables 
typedef struct {
	float x, y, vx, vy; // position and velocity of particle
} Particle;
int num_particles = 1000;
std::vector<Particle> particles(num_particles); 
Vector2 max_velocity(20,-50);
Vector2 min_velocity(-20, -100);
void reset_particle(Particle* p,float x, float y){
		p->x = x;
		p->y = y;
		p->vx = randomValue() * (max_velocity.x - min_velocity.x) + min_velocity.x;
		p->vy = randomValue() * (max_velocity.y - min_velocity.y) + min_velocity.y;
}

// task 6 variables
Image toolbar;
Image canvas;
Vector2 prev_mouse_pos;
Color draw_color(0, 0, 0);

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
	// compute formulas
	grayscale_image.forEachPixel([](Color c) {float rgb = (c.r + c.g + c.b) / 3.0f; return Color(rgb, rgb, rgb); });
	channel_swap_image.forEachPixel([](Color c) {return Color(c.b, c.r, c.g); });

	//loading task4.tga
	task4.loadTGA("./task4.tga");
	task4.scale(window_width, window_height);
	task4.flipY();
	smalltask4.loadTGA("./task4.tga");
	smalltask4.scale(window_width / 5, window_height / 5);
	smalltask4.flipY();

	// initializing particles
	for (int i = 0; i < particles.size(); i++){
		reset_particle(&(particles[i]), randomValue() * window_width, randomValue() * window_height);
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
		framebuffer.drawCircle(100, 100, 100, Color::RED);
		framebuffer.drawCircle(300, 100, 100, Color::YELLOW);
		framebuffer.drawCircle(500, 100, 100, Color::GREEN);
		framebuffer.drawCircle(700, 100, 100, Color::BLUE);

		framebuffer.drawRectangle(50, 300, 50, 200, Color::WHITE, 0);
		framebuffer.drawRectangle(100, 375, 50, 50, Color::WHITE, 0);
		framebuffer.drawRectangle(150, 300, 50, 200, Color::WHITE, 0);
		framebuffer.drawCircleLines(325, 400, 100, Color::WHITE, 200);
		framebuffer.drawRectangle(450, 300, 50, 200, Color::WHITE, 1);
		framebuffer.drawRectangle(500, 300, 75, 50, Color::WHITE, 1);
		framebuffer.drawLine(600, 300, 675, 500, Color::WHITE);
		framebuffer.drawLine(750, 300, 675, 500, Color::WHITE);
		framebuffer.drawLine(637.5, 400, 712.5, 400, Color::WHITE);

		framebuffer.drawLine(285, 410, 285, 450, Color::WHITE);
		framebuffer.drawLine(365, 410, 365, 450, Color::WHITE);
		framebuffer.drawCircleLines_Part(325, 400, 75, PI, PI * 2, Color::CYAN, 100);
		break;
		}
	case 2: { // task 2
		float radius = (framebuffer.height * framebuffer.height); //+ framebuffer.width * framebuffer.width); 
		for (int x = 0; x < framebuffer.width; x++)
			for (int y = 0; y < framebuffer.height; y++)
				if (formula_shown == 0) {
					Color c = ((x / 20) % 2) == ((y / 20) % 2) ? Color::BLACK : Color::WHITE;
					framebuffer.setPixel(x, y, c);
				}
				else {
					Color c = Color::lerp(Color::BLUE, Color::RED, ((float)x) / framebuffer.width);
					framebuffer.setPixel(x, y, c);
				}
		break;
	}
	case 3: { // task 3
		if (filter_shown == 0) { // normal image
			framebuffer = normal_image;
		}
		else if (filter_shown == 1) { // grayscale
			framebuffer = grayscale_image;
		}
		else { // channel swap
			framebuffer = channel_swap_image;
		}
		break;
	}
	case 4: { // task4
		for (unsigned int x = 0; x < window_width; x++) {
			for (unsigned int y = 0; y < window_height; y++) {
				int startx = window_width / 2;
				int starty = window_height / 2;
				int newx = x * cos(angle) - y * sin(angle) - startx * cos(angle) + starty * sin(angle) + startx;
				int newy = y * cos(angle) + x * sin(angle) - starty * cos(angle) - startx * sin(angle) + starty;
				int scalex = window_width * (scale - 1) / (scale * 2);
				int scaley = window_height * (scale - 1) / (scale * 2);
				framebuffer.setPixelSafe(newx, newy, task4.getPixelSafe(x * (1 / std::abs(scale)) + scalex, y * (1 / std::abs(scale)) + scaley));
			}
		}
		break;
	}
	case 5: { // task 5
		for (Particle p : particles)
			framebuffer.setPixelSafe(p.x, p.y, Color::WHITE);
		break;
	}
	case 6: { //task 6
		framebuffer = canvas;
		for (int xi = 0; xi < toolbar.width; xi++)
			for (int yi = 0; yi < toolbar.height; yi++)
				framebuffer.setPixelSafe(xi, framebuffer.height - yi, toolbar.getPixel(xi, yi));
	}
	}
}

//called after render
void Application::update(double seconds_elapsed) {
	switch (app_state) {
	case 6: { // task 6
		if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if (mouse_position.y > window_height - toolbar.height) {
				// check what icon has been clicked
				if (mouse_position.x < 50) // first icon: clear canvas
					canvas.fill(Color::WHITE);
				else if (mouse_position.x < 100) { // second icon: save canvas
					canvas.flipY(); // flip before saving so image gets saved correctly
					canvas.saveTGA("./BeatifulDrawing.tga");
					canvas.flipY();
				}
				else if (mouse_position.x < 500) { // color icons: pick correct icon
					int cy = 25;
					int cx = snap(mouse_position.x - 25, 50.0f) + 25;
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
	case 4: { // task4 use WASD keys to rotate/ scale
		if (keystate[SDL_SCANCODE_W])
			scale += seconds_elapsed * scale_velocity;
		if (keystate[SDL_SCANCODE_S])
			scale -= seconds_elapsed * scale_velocity;
		if (keystate[SDL_SCANCODE_A])
			angle += seconds_elapsed * angular_velocity;
		if (keystate[SDL_SCANCODE_D])
			angle -= seconds_elapsed * angular_velocity;
		break;
	}
	case 5: { // task5
		// update particles:
		Particle* p;
		for (int i = 0; i < particles.size(); i++) {
			p = &particles[i];
			p->x += p->vx * seconds_elapsed;
			p->y += p->vy * seconds_elapsed;
			if (p->x < 0 || p->x > window_width || p->y < 0 || p->y > window_height)
				reset_particle(p, randomValue() * window_width, window_height - 1);
		}
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
		formula_shown = (formula_shown + 1) % 2;
		filter_shown = 0;
		break;
	case SDL_SCANCODE_W:
		filter_shown = 1;
		break;
	case SDL_SCANCODE_E:
		filter_shown = 2;
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


