#include "application.h"
#include "utils.h"
#include "includes.h"
#include "utils.h"

#include "image.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "material.h"
#include "light.h"


Camera* camera = NULL;
Mesh* mesh = NULL;
Mesh* light_mesh = NULL;
Shader* shader = NULL;

//might be useful...
Material* material = NULL;
Light* light = NULL;
std::vector<Light*>* lights = NULL; 
int controlled_light_index = 0;
float light_speed = 20;
const float z_light_pos = 10;
const float light_intensity = 100;
float camera_speed = 20;

Shader* light_sphere_shader = NULL;
Shader* phong_shader = NULL;
Shader* gouraud_shader = NULL;


Vector3 ambient_light(0.1,0.1,0.1); //here we can store the global ambient light of the scene

float angle = 0;
float orbit_angle = 0;

// Variables
float seconds = 0;

// application variables
int selected_shader = 1;
int mesh_num = 1;
float mesh_offset = 20.0f;

// useful function declaration
void passLightInfoToShader(Light* light, Shader* shader);
void passMaterialInfoToShader(Material* light, Shader* shader);



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
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;
	
	//here we create a global camera and set a position and projection properties
	camera = new Camera();
	camera->lookAt(Vector3(0,20,20),Vector3(0,10,0),Vector3(0,1,0));
	camera->setPerspective(60,window_width / window_height,0.1,10000);

	//then we load a mesh
	mesh = new Mesh();
	if( !mesh->loadOBJ( "../res/meshes/lee.obj" ) )
		std::cout << "FILE Lee.obj NOT FOUND " << std::endl;
	
	light_mesh = new Mesh();
	if (!light_mesh->loadOBJ("../res/meshes/sphere.obj"))
		std::cout<<"FILE sphere.obj NOT FOUND"<<std::endl;

	light_sphere_shader = Shader::Get("../res/shaders/simple.vs", "../res/shaders/simple.fs");
	gouraud_shader = Shader::Get("../res/shaders/gouraud.vs", "../res/shaders/gouraud.fs");
	phong_shader = Shader::Get("../res/shaders/phong.vs", "../res/shaders/phong.fs");

	//CODE HERE:
	//create a light (or several) and and some materials
	light = new Light();
	light->position.set(0, 15, z_light_pos);
	light->diffuse_color.random(light_intensity);
	light->specular_color = light->diffuse_color;

	lights = new std::vector<Light*>();
	lights->push_back(light);

	material = new Material();
	material->shininess = 5;
}

//render one frame
void Application::render(void)
{
	//update the aspect of the camera acording to the window size
	camera->aspect = window_width / window_height;
	camera->updateProjectionMatrix();
	//Get the viewprojection matrix from our camera
	Matrix44 viewprojection = camera->getViewProjectionMatrix();
	
	//set the clear color of the colorbuffer as the ambient light so it matches
	glClearColor(ambient_light.x, ambient_light.y, ambient_light.z, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear framebuffer and depth buffer 
	glEnable( GL_DEPTH_TEST ); //enable depth testing for occlusions
	glDepthFunc(GL_LEQUAL); //Z will pass if the Z is LESS or EQUAL to the Z of the pixel

	// render light ball
	Matrix44 light_model_matrix;
	light_sphere_shader->enable();

	for (Light *l : *lights){
		light_model_matrix.setIdentity();
		light_model_matrix.translate(l->position.x, l->position.y, l->position.z); //example of translation
		light_sphere_shader->setMatrix44("viewprojection", viewprojection); //upload viewprojection info to the shader
		light_sphere_shader->setMatrix44("model", light_model_matrix);
		light_sphere_shader->setVector3("light_color", l->diffuse_color);
		light_mesh->render(GL_TRIANGLES);
	}

	light_sphere_shader->disable();

	// proceed to render mesh
	Matrix44 model_matrix;
	model_matrix.setIdentity();
	model_matrix.translate(0, 0, 0); //example of translation
	model_matrix.rotate(angle, Vector3(0, 1, 0));

	// choose shader
	switch (selected_shader) {
		case 1: shader = gouraud_shader; break;
		case 2: shader = phong_shader; break;
	}

	// enable shader 
	shader->enable();

	// pass values to shader
	shader->setMatrix44("viewprojection", viewprojection); //upload viewprojection info to the shader
	shader->setVector3("light_amb", ambient_light);
	shader->setVector3("eye_pos", camera->eye);
	if (lights->size() > 0)
		passLightInfoToShader(lights->at(0), shader);
	passMaterialInfoToShader(material, shader);

	// draw with the first light 
	glDisable(GL_BLEND);
	for (int mesh_index = 0; mesh_index < mesh_num; mesh_index++){
		model_matrix.m[12] = mesh_index * mesh_offset;
		shader->setMatrix44("model", model_matrix); //upload the transform matrix to the shader
		mesh->render(GL_TRIANGLES);
	}

	// if multiuple lights draw 
	if (lights->size() > 1){
		glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE );
		for (int i = 1; i < lights->size(); i++){
			passLightInfoToShader(lights->at(i), shader);
			for (int mesh_index = 0; mesh_index < mesh_num; mesh_index++){
				model_matrix.m[12] = mesh_index * mesh_offset;
				//model_matrix.setTranslation(mesh_index * mesh_offset, 0, 0);
				shader->setMatrix44("model", model_matrix); //upload the transform matrix to the shader
				mesh->render(GL_TRIANGLES);
			}
		}
	}
	//disable shader when we do not need it any more
	shader->disable();

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

//called after render
void Application::update(double seconds_elapsed)
{
	if (keystate[SDL_SCANCODE_SPACE])
		angle += seconds_elapsed;

	if (keystate[SDL_SCANCODE_RIGHT])
		camera->eye = camera->eye + Vector3(1, 0, 0) * seconds_elapsed * camera_speed;
	else if (keystate[SDL_SCANCODE_LEFT])
		camera->eye = camera->eye + Vector3(-1, 0, 0) * seconds_elapsed * camera_speed;
	if (keystate[SDL_SCANCODE_UP])
		camera->eye = camera->eye + Vector3(0, 1, 0) * seconds_elapsed * camera_speed;
	else if (keystate[SDL_SCANCODE_DOWN])
		camera->eye = camera->eye + Vector3(0, -1, 0) * seconds_elapsed * camera_speed;

	// light controls
	if (keystate[SDL_SCANCODE_W])
		lights->at(controlled_light_index)->position.y += seconds_elapsed * light_speed;
	else if (keystate[SDL_SCANCODE_S])
		lights->at(controlled_light_index)->position.y -= seconds_elapsed * light_speed;
	if (keystate[SDL_SCANCODE_D])
		lights->at(controlled_light_index)->position.x += seconds_elapsed * light_speed;
	else if (keystate[SDL_SCANCODE_A])
		lights->at(controlled_light_index)->position.x -= seconds_elapsed * light_speed;
	if (keystate[SDL_SCANCODE_F])
		lights->at(controlled_light_index)->position.z += seconds_elapsed * light_speed;
	else if (keystate[SDL_SCANCODE_G])
		lights->at(controlled_light_index)->position.z -= seconds_elapsed * light_speed;

	// orbit
	if (keystate[SDL_SCANCODE_J])
		camera->orbit(orbit_angle + seconds_elapsed, Vector3(0, 1, 0));
	else if (keystate[SDL_SCANCODE_L])
		camera->orbit(orbit_angle - seconds_elapsed, Vector3(0, 1, 0));
	if (keystate[SDL_SCANCODE_I])
		camera->orbit(orbit_angle + seconds_elapsed, Vector3(1, 0, 0));
	else if (keystate[SDL_SCANCODE_K])
		camera->orbit(orbit_angle - seconds_elapsed, Vector3(1, 0, 0));
}

//keyboard press event 
void Application::onKeyPressed( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_1: selected_shader = 1; std::cout<<"Switched shader to gouraud shader\n"<<std::endl; break;
		case SDLK_2: selected_shader = 2; std::cout<<"Switched shader to phong shader\n"<<std::endl; break;
		case SDLK_PLUS: mesh_num++; break;
		case SDLK_MINUS: mesh_num--; break;
		case SDLK_x:{ // delete a light
			if (lights->size() <= 1) break;
			Light* tmp = lights->at(controlled_light_index);
			lights->erase(lights->begin() + controlled_light_index);
			delete tmp;
			break;
		}
		case SDLK_z: controlled_light_index++; break;
		case SDLK_c: { // create a new light
			Light* tmp = new Light();
			tmp->position.set(0,15,z_light_pos);
			tmp->diffuse_color.random(light_intensity);
			tmp->specular_color = tmp->diffuse_color;
			controlled_light_index = lights->size();
			lights->push_back(tmp);
			break;
		}
		case SDLK_ESCAPE: exit(0); break; //ESC key, kill the app
		case SDLK_r: 
			Shader::ReloadAll();
			break; //ESC key, kill the app
	}

	if (mesh_num < 1) mesh_num = 1;
	else if (mesh_num > 30) mesh_num = 30;

	controlled_light_index %= lights->size();
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

// helper function

void passLightInfoToShader(Light* light, Shader* shader){
	shader->setVector3("light_pos", light->position);
	shader->setVector3("light_dif", light->diffuse_color);
	shader->setVector3("light_spc", light->specular_color);
}

void passMaterialInfoToShader(Material* material, Shader* shader){
	shader->setVector3("material_dif", material->diffuse);
	shader->setVector3("material_spc", material->specular);
	shader->setVector3("material_amb", material->ambient);
	shader->setFloat("material_shin", material->shininess);
}
