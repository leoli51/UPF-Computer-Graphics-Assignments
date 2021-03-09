#include "application.h"
#include "utils.h"
#include "image.h"
#include "mesh.h"

// application variables
int app_state = 1;

// camera position variables
const float move_velocity = 50;
const float look_velocity = 10;
const float fov_velocity = 5;
const float min_fov = 5, max_fov = 120;

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
	camera->lookAt(Vector3(0, 0, 40), Vector3(0, 0, 0), Vector3(0, 1, 0)); //define eye,center,up
	camera->perspective(60, window_width / (float)window_height, 0.1, 10000); //define fov,aspect,near,far

	//load a mesh
	mesh = new Mesh();
	if (!mesh->loadOBJ("lee.obj"))
		std::cout << "FILE Lee.obj NOT FOUND" << std::endl;

	//load the texture
	texture = new Image();
	texture->loadTGA("color.tga");
}

//render one frame
void Application::render(Image& framebuffer)
{
	framebuffer.fill(Color(40, 45, 60)); //clear

	//set zbuffer
	float max_depth = -(camera->far_plane + 1);
	for (int x = 0; x < zbuffer.width; x++){
		for (int y = 0; y < zbuffer.height; y++){
			//give a huge number
			zbuffer.setPixel(x, y, max_depth);
		}
	}
	// set light 
	Vector3 light_pos(50, 50, 0);
	Vector3 light_dif(0.6f, 0.6f, 0.6f);
	Vector3 light_spc(0.6f, 0.6f, 0.6f);
	Vector3 light_amb(0.1, 0.1, 0.1);

	// set material
	Vector3 material_dif(1, 1, 1);
	Vector3 material_spc(1, 1, 1);
	Vector3 material_amb(1, 1, 1);
	float material_shin = 30.0;

	// set up
	Vector3 eye_pos = camera->eye;
	Matrix44 model_matrix;
	model_matrix.setIdentity();
	Matrix44 T;
	T.setTranslation(0, 0, 0);
	model_matrix = model_matrix * T;
	Matrix44 viewprojection = camera->getViewProjectionMatrix();

	// start illumination
	for (int i = 0; i < mesh->vertices.size(); i += 3)
	{
		Vector3 v0 = mesh->vertices[i]; //extract vertex from mesh
		Vector3 v1 = mesh->vertices[i + 1]; //extract vertex from mesh
		Vector3 v2 = mesh->vertices[i + 2]; //extract vertex from mesh
		Vector3 v0_normal(v0.y, -v0.x, 0);
		Vector3 v1_normal(v1.y, -v1.x, 0);
		Vector3 v2_normal(v2.y, -v2.x, 0);
		//convert local coordinate to world coordinates
		Vector3 wPos_0 = (model_matrix * Vector4(v0.x, v0.y, v0.z, 1.0)).getVector3();
		Vector3 wPos_1 = (model_matrix * Vector4(v1.x, v1.y, v1.z, 1.0)).getVector3();
		Vector3 wPos_2 = (model_matrix * Vector4(v2.x, v2.y, v2.z, 1.0)).getVector3();
		//convert local normal to world coordinates
		Vector3 wNormal_0 = (model_matrix * Vector4(v0_normal.x, v0_normal.y, v0_normal.z, 1.0)).getVector3();
		Vector3 wNormal_1 = (model_matrix * Vector4(v1_normal.x, v1_normal.y, v1_normal.z, 1.0)).getVector3();
		Vector3 wNormal_2 = (model_matrix * Vector4(v2_normal.x, v2_normal.y, v2_normal.z, 1.0)).getVector3();

		//project the vertex by the model view projection 
		Vector4 Position_0 = viewprojection * Vector4(wPos_0.x, wPos_0.y, wPos_0.z, 1.0);
		Vector4 Position_1 = viewprojection * Vector4(wPos_1.x, wPos_1.y, wPos_1.z, 1.0);
		Vector4 Position_2 = viewprojection * Vector4(wPos_2.x, wPos_2.y, wPos_2.z, 1.0);

		Vector3 L0(light_pos - wPos_0); L0.normalize();
		Vector3 L1(light_pos - wPos_1); L1.normalize();
		Vector3 L2(light_pos - wPos_2); L2.normalize();
		
		Vector3 N0 = wNormal_0.normalize();
		Vector3 N1 = wNormal_1.normalize();
		Vector3 N2 = wNormal_2.normalize();
		
		//Vector3 R = normalize(reflect(-L, N));

		Vector3 V0(eye_pos - wPos_0); V0.normalize();
		Vector3 V1(eye_pos - wPos_0); V0.normalize();
		Vector3 V2(eye_pos - wPos_0); V0.normalize();


		float LdotN_0 = max(0.0, L0.dot(N0));
		float LdotN_1 = max(0.0, L1.dot(N1));
		float LdotN_2 = max(0.0, L2.dot(N2));
		//float RdotV = max(0.0, R0.dot(V0));
		//RdotV = pow(RdotV, material_shin);

		float dst_squared_0 = wPos_0.distance(light_pos);
		float dst_squared_1 = wPos_1.distance(light_pos);
		float dst_squared_2 = wPos_2.distance(light_pos);

		dst_squared_0 = dst_squared_0 * dst_squared_0;
		dst_squared_1 = dst_squared_1 * dst_squared_1;
		dst_squared_2 = dst_squared_2 * dst_squared_2;

		Vector3 amb(light_amb.x * material_amb.x, light_amb.y * material_amb.y, light_amb.z * material_amb.z);

		Vector3 dif_0(light_dif.x / dst_squared_0 * LdotN_0 * material_dif.x, light_dif.y / dst_squared_0 * LdotN_0 * material_dif.y, light_dif.z / dst_squared_0 * LdotN_0 * material_dif.z);
		Vector3 dif_1(light_dif.x / dst_squared_1 * LdotN_1 * material_dif.x, light_dif.y / dst_squared_1 * LdotN_1 * material_dif.y, light_dif.z / dst_squared_1 * LdotN_1 * material_dif.z);
		Vector3 dif_2(light_dif.x / dst_squared_2 * LdotN_2 * material_dif.x, light_dif.y / dst_squared_2 * LdotN_2 * material_dif.y, light_dif.z / dst_squared_2 * LdotN_2 * material_dif.z);
		
		//Vector3 spc = light_spc / dst_squared * RdotV * material_spc;

		//compute color
		Vector3 color0 = amb + dif_0; //+ spc;
		Vector3 color1 = amb + dif_1; //+ spc;
		Vector3 color2 = amb + dif_2; //+ spc;

		//set the ouput color por the pixel
		Vector4 FragColor0 = Vector4(color0.x, color0.y, color0.z, 1.0);
		Vector4 FragColor1 = Vector4(color1.x, color1.y, color1.z, 1.0);
		Vector4 FragColor2 = Vector4(color2.x, color2.y, color2.z, 1.0);

		framebuffer.fillTriangleWithColor(Position_0.getVector3(), Position_1.getVector3(), Position_2.getVector3(), FragColor0.getVector3(), FragColor1.getVector3(), FragColor2.getVector3(), zbuffer);


	}



	////for every point of the mesh (to draw triangles take three points each time and connect the points between them (1,2,3,   4,5,6,   ...)
	//for (int i = 0; i < mesh->vertices.size(); i += 3)
	//{
	//	Vector3 v0 = mesh->vertices[i]; //extract vertex from mesh
	//	Vector3 v1 = mesh->vertices[i + 1]; //extract vertex from mesh
	//	Vector3 v2 = mesh->vertices[i + 2]; //extract vertex from mesh

	//	//Vector2 texcoord = mesh->uvs[i]; //texture coordinate of the vertex (they are normalized, from 0,0 to 1,1)
	//	//project every point in the mesh to normalized coordinates using the viewprojection_matrix inside camera
	//	v0 = camera->projectVector(v0);
	//	v1 = camera->projectVector(v1);
	//	v2 = camera->projectVector(v2);

	//	//convert from normalized (-1 to +1) to framebuffer coordinates (0,W)
	//	v0.x = (v0.x + 1.0) / 2.0 * framebuffer.width;
	//	v1.x = (v1.x + 1.0) / 2.0 * framebuffer.width;
	//	v2.x = (v2.x + 1.0) / 2.0 * framebuffer.width;
	//	v0.y = (v0.y + 1.0) / 2.0 * framebuffer.height;
	//	v1.y = (v1.y + 1.0) / 2.0 * framebuffer.height;
	//	v2.y = (v2.y + 1.0) / 2.0 * framebuffer.height;

	//	//paint points in framebuffer (using your drawTriangle function or the fillTriangle function)
	//	switch (app_state) {
	//	case 1:
	//		framebuffer.drawLineBresenham(v0.x, v0.y, v1.x, v1.y, Color::WHITE);
	//		framebuffer.drawLineBresenham(v1.x, v1.y, v2.x, v2.y, Color::WHITE);
	//		framebuffer.drawLineBresenham(v2.x, v2.y, v0.x, v0.y, Color::WHITE);
	//		break;
	//	case 2:
	//		framebuffer.fillTriangleWithColor(v0, v1, v2, Color::RED, Color::GREEN, Color::BLUE, zbuffer);
	//		break;
	//	case 3:
	//		framebuffer.fillTriangleWithTexture(v0, v1, v2, mesh->uvs[i], mesh->uvs[i + 1], mesh->uvs[i + 2], *texture, zbuffer);
	//		break;
	//	}
	//}
	//std::cout<<"Finished!"<<std::endl;

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
	Vector3 move_vector(0, 0, 0);
	Vector3 center_move_vector(0, 0, 0);
	if (keystate[SDL_SCANCODE_LEFT])
		move_vector.x = -1;
	if (keystate[SDL_SCANCODE_RIGHT])
		move_vector.x = 1;
	if (keystate[SDL_SCANCODE_DOWN])
		move_vector.y = -1;
	if (keystate[SDL_SCANCODE_UP])
		move_vector.y = 1;
	//if (keystate[SDL_SCANCODE_Z])
	//	move_vector.z = -1;
	//if (keystate[SDL_SCANCODE_X])
	//	move_vector.z = 1;
	if (keystate[SDL_SCANCODE_D])
		center_move_vector.x = 1;
	if (keystate[SDL_SCANCODE_A])
		center_move_vector.x = -1;
	if (keystate[SDL_SCANCODE_S])
		center_move_vector.y = -1;
	if (keystate[SDL_SCANCODE_W])
		center_move_vector.y = 1;
	if (keystate[SDL_SCANCODE_F])
		camera->fov += seconds_elapsed * fov_velocity;
	if (keystate[SDL_SCANCODE_G])
		camera->fov -= seconds_elapsed * fov_velocity;

	camera->fov = clamp(camera->fov, min_fov, max_fov);
	camera->eye += move_vector * move_velocity * seconds_elapsed;
	camera->center += center_move_vector * look_velocity * seconds_elapsed;
	//camera->center = camera->view_matrix.frontVector() + camera->eye;


	//if we modify the camera fields, then update matrices
	camera->updateViewMatrix();
	camera->updateProjectionMatrix();
}

//keyboard press event 
void Application::onKeyDown(SDL_KeyboardEvent event)
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch (event.keysym.scancode)
	{
	case SDL_SCANCODE_1: app_state = 1; break;
	case SDL_SCANCODE_2: app_state = 2; break;
	case SDL_SCANCODE_3: app_state = 3; break;

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
void Application::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{

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
