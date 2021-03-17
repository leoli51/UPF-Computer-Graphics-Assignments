#include "application.h"
#include "utils.h"
#include "image.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "light.h"
#include "material.h"

void passLightInfoToShader(Light* light, Shader* shader);
void passMaterialInfoToShader(Material* light, Shader* shader);
void renderLightBall(Light* l, Matrix44* viewproj);

Camera* camera = NULL;
Mesh* mesh = NULL;
Mesh* light_mesh = NULL;
Matrix44 model_matrix;
Shader* shader = NULL;
Shader* diffuse = NULL;
Shader* specular = NULL;
Shader* normals = NULL;
Shader* material_shader = NULL;
Shader* light_sphere_shader = NULL;

Texture* texture = NULL;
Texture* normal_texture = NULL;

Light* light = NULL;
Material materials[3];

float mesh_angle = 0;
float rotation_speed = 2;

float camera_speed = 20;
const float fov_velocity = 20;
const float min_fov = 5, max_fov = 120;

float light_speed = 20;

const Vector3 ambient_light = Vector3(0.2,0.2,0.2);
const int mesh_num = 3;
const int mesh_offset = 25;

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
	camera->lookAt(Vector3(mesh_offset,20,25),Vector3(mesh_offset,10,0),Vector3(0,1,0));
	camera->setPerspective(60,window_width / window_height,0.1,10000);

	//then we load a mesh
	mesh = new Mesh();
	mesh->loadOBJ("../res/meshes/lee.obj");
	light_mesh = new Mesh();
	light_mesh->loadOBJ("../res/meshes/sphere.obj");

	light = new Light();
	light->position.set(mesh_offset,10,15);
	light->diffuse_color.set(100,100,100);
	light->specular_color = light->diffuse_color;

	materials[0] = Material();
	materials[1] = Material();
	materials[2] = Material();
	materials[1].diffuse.set(1,.8, 0.8);
	materials[1].specular = materials[1].diffuse;
	materials[1].ambient = materials[1].diffuse;
	materials[2].diffuse.set(0.8,0.8,1);
	materials[2].specular = materials[2].diffuse;
	materials[2].ambient = materials[2].diffuse;

	//load the texture
	texture = new Texture();
	if(!texture->load("../res/textures/lee_color_specular.tga"))
	{
		std::cout << "Texture not found" << std::endl;
		exit(1);
	}

	normal_texture = new Texture();
	if (!normal_texture->load("../res/textures/lee_normal.tga")){
		std::cout << "Texture not found" << std::endl;
		exit(1);
	}

	//we load our shaders
	diffuse = Shader::Get("../res/shaders/phong_diffuse.vs","../res/shaders/phong_diffuse.fs");
	specular = Shader::Get("../res/shaders/phong_specular.vs", "../res/shaders/phong_specular.fs");
	normals = Shader::Get("../res/shaders/phong_normals.vs", "../res/shaders/phong_normals.fs");
	material_shader = Shader::Get("../res/shaders/phong_material.vs", "../res/shaders/phong_material.fs");
	light_sphere_shader = Shader::Get("../res/shaders/simple.vs", "../res/shaders/simple.fs");

	shader = diffuse;
}

//render one frame
void Application::render(void)
{
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable( GL_DEPTH_TEST );

	//Get the viewprojection
	camera->aspect = window_width / window_height;
	Matrix44 viewprojection = camera->getViewProjectionMatrix();

	// render light balls
	renderLightBall(light, &viewprojection);

	// set the model matrix for all meshes (could be done for each mesh)
	model_matrix.setIdentity();
	model_matrix.rotate(mesh_angle, Vector3(0, 1, 0));

	//enable the shader
	shader->enable();
	shader->setMatrix44("viewprojection", viewprojection); //upload info to the shader
	shader->setVector3("eye_pos", camera->eye);
	shader->setVector3("light_amb", ambient_light);
	shader->setTexture("color_texture", texture, 0); //set texture in slot 0
	shader->setTexture("normal_texture", normal_texture, 1);

	passLightInfoToShader(light, shader);

	//render the data
	for (int mesh_index = 0; mesh_index < mesh_num; mesh_index++){
		passMaterialInfoToShader(&materials[mesh_index], shader);
		model_matrix.m[12] = mesh_index * mesh_offset;
		shader->setMatrix44("model", model_matrix); //upload the transform matrix to the shader
		mesh->render(GL_TRIANGLES);
	}
	//disable shader
	shader->disable();

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

//called after render
void Application::update(double seconds_elapsed)
{
	if (keystate[SDL_SCANCODE_SPACE])
	{
		//model_matrix.rotateLocal(seconds_elapsed,Vector3(0,1,0));
		mesh_angle += seconds_elapsed * rotation_speed;
	}

	// camera controls
	if (keystate[SDL_SCANCODE_RIGHT])
		camera->eye = camera->eye + Vector3(1, 0, 0) * seconds_elapsed * camera_speed;
	else if (keystate[SDL_SCANCODE_LEFT])
		camera->eye = camera->eye + Vector3(-1, 0, 0) * seconds_elapsed * camera_speed;
	if (keystate[SDL_SCANCODE_UP])
		camera->eye = camera->eye + Vector3(0, 1, 0) * seconds_elapsed * camera_speed;
	else if (keystate[SDL_SCANCODE_DOWN])
		camera->eye = camera->eye + Vector3(0, -1, 0) * seconds_elapsed * camera_speed;
	if (keystate[SDL_SCANCODE_F])
		camera->fov += seconds_elapsed * fov_velocity;
	if (keystate[SDL_SCANCODE_G])
		camera->fov -= seconds_elapsed * fov_velocity;

	camera->fov = clamp(camera->fov, min_fov, max_fov);
	//if we modify the camera fields, then update matrices
	camera->updateViewMatrix();
	camera->updateProjectionMatrix();

	// light controls
	if (keystate[SDL_SCANCODE_W])
		light->position.y += seconds_elapsed * light_speed;
	else if (keystate[SDL_SCANCODE_S])
		light->position.y -= seconds_elapsed * light_speed;
	if (keystate[SDL_SCANCODE_D])
		light->position.x += seconds_elapsed * light_speed;
	else if (keystate[SDL_SCANCODE_A])
		light->position.x -= seconds_elapsed * light_speed;
}

//keyboard press event 
void Application::onKeyPressed( SDL_KeyboardEvent event )
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode){
		case SDL_SCANCODE_1: shader = diffuse; break;
		case SDL_SCANCODE_2: shader = specular; break;
		case SDL_SCANCODE_3: shader = normals; break; 
		case SDL_SCANCODE_4: shader = material_shader; break;
		case SDL_SCANCODE_8: camera->lookAt(camera->eye, Vector3(0,10,0), Vector3(0,1,0)); break;
		case SDL_SCANCODE_9: camera->lookAt(camera->eye, Vector3(mesh_offset,10,0), Vector3(0,1,0)); break;
		case SDL_SCANCODE_0: camera->lookAt(camera->eye, Vector3(mesh_offset*2,10,0), Vector3(0,1,0)); break;
		case SDL_SCANCODE_R: Shader::ReloadAll(); break;
        case SDL_SCANCODE_ESCAPE: exit(0); break; //ESC key, kill the app
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


void passLightInfoToShader(Light* light, Shader* shader){
	shader->setVector3("light_pos", light->position);
	shader->setVector3("light_dif", light->diffuse_color);
	shader->setVector3("light_spc", light->specular_color);
}


void passMaterialInfoToShader(Material* material, Shader* shader){
	shader->setVector3("mat_dif", material->diffuse);
	shader->setVector3("mat_spc", material->specular);
	shader->setVector3("mat_amb", material->ambient);
	shader->setFloat("mat_shin", material->shininess);
}

void renderLightBall(Light* l, Matrix44* viewprojection){
	// render light ball
	Matrix44 light_model_matrix;
	light_sphere_shader->enable();

	light_model_matrix.setIdentity();
	light_model_matrix.translate(l->position.x, l->position.y, l->position.z); //example of translation
	light_sphere_shader->setMatrix44("viewprojection", *viewprojection); //upload viewprojection info to the shader
	light_sphere_shader->setMatrix44("model", light_model_matrix);
	light_sphere_shader->setVector3("light_color", l->diffuse_color);
	light_mesh->render(GL_TRIANGLES);

	light_sphere_shader->disable();
}

