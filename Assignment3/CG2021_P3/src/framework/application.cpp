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
	if (!texture->loadTGA("lee_color_specular.tga")) {
		std::cout << "Texture not found" << std::endl;
		exit(1);
	}

	normal = new Image();
	if (!normal->loadTGA("lee_normal.tga")) {
		std::cout << "Texture not found" << std::endl;
		exit(1);
	}
}

//render one frame
void Application::render(Image& framebuffer)
{
	framebuffer.fill(Color(40, 45, 60)); //clear

	//set zbuffer
	float max_depth = -(camera->far_plane + 1);
	for (int x = 0; x < zbuffer.width; x++) {
		for (int y = 0; y < zbuffer.height; y++) {
			//give a huge number
			zbuffer.setPixel(x, y, max_depth);
		}
	}
	// set light 
	Vector3 light_pos(50, 50, 0);
	Vector3 light_dif(0.6f, 0.6f, 0.6f);
	Vector3 light_spc(0.6f, 0.6f, 0.6f);
	Vector3 light_amb(0.1, 0.1, 0.1);

	// set up
	Vector3 eye_pos = camera->eye;
	Matrix44 model_matrix;
	model_matrix.setIdentity();
	model_matrix.traslate(0, 0, 0);
	Matrix44 viewprojection = camera->getViewProjectionMatrix();

	// start illumination
	for (int i = 0; i < mesh->vertices.size(); i += 3)
	{
		Vector3 p0 = mesh->vertices[i]; //extract vertex from mesh
		Vector3 p1 = mesh->vertices[i + 1]; //extract vertex from mesh
		Vector3 p2 = mesh->vertices[i + 2]; //extract vertex from mesh

		Vector3 v0 = camera->projectVector(p0);
		Vector3 v1 = camera->projectVector(p1);
		Vector3 v2 = camera->projectVector(p2);

		//convert from normalized (-1 to +1) to framebuffer coordinates (0,W)
		v0.x = (v0.x + 1.0) / 2.0 * framebuffer.width;
		v1.x = (v1.x + 1.0) / 2.0 * framebuffer.width;
		v2.x = (v2.x + 1.0) / 2.0 * framebuffer.width;
		v0.y = (v0.y + 1.0) / 2.0 * framebuffer.height;
		v1.y = (v1.y + 1.0) / 2.0 * framebuffer.height;
		v2.y = (v2.y + 1.0) / 2.0 * framebuffer.height;

		// get normal
		Vector3 v0_normal((float)normal->getPixel(mesh->uvs[i].normalize().x * normal->width, mesh->uvs[i].y * normal->height).r / 255, (float)normal->getPixel(mesh->uvs[i].x, mesh->uvs[i].y).g / 255, (float)normal->getPixel(mesh->uvs[i].x, mesh->uvs[i].y).b / 255);
		Vector3 v1_normal((float)normal->getPixel(mesh->uvs[i + 1].x * normal->width, mesh->uvs[i + 1].y * normal->height).r / 255, (float)normal->getPixel(mesh->uvs[i + 1].x * normal->width, mesh->uvs[i + 1].y * normal->height).g / 255, (float)normal->getPixel(mesh->uvs[i + 1].x * normal->width, mesh->uvs[i + 1].y * normal->height).b / 255);
		Vector3 v2_normal((float)normal->getPixel(mesh->uvs[i + 2].x * normal->width, mesh->uvs[i + 2].y * normal->height).r / 255, (float)normal->getPixel(mesh->uvs[i + 2].x * normal->width, mesh->uvs[i + 2].y * normal->height).g / 255, (float)normal->getPixel(mesh->uvs[i + 2].x * normal->width, mesh->uvs[i + 2].y * normal->height).b / 255);

		// convert from -1,1 to 0,1
		v0_normal.set(v0_normal.x * 0.5 + 0.5, v0_normal.y * 0.5 + 0.5, v0_normal.z * 0.5 + 0.5);
		v1_normal.set(v1_normal.x * 0.5 + 0.5, v1_normal.y * 0.5 + 0.5, v1_normal.z * 0.5 + 0.5);
		v2_normal.set(v2_normal.x * 0.5 + 0.5, v2_normal.y * 0.5 + 0.5, v2_normal.z * 0.5 + 0.5);

		// compute color texture
		Vector3 texture0((float)texture->getPixel(mesh->uvs[i].x * texture->width, mesh->uvs[i].y * texture->height).r / 255, (float)texture->getPixel(mesh->uvs[i].x * texture->width, mesh->uvs[i].y * texture->height).g / 255, (float)texture->getPixel(mesh->uvs[i].x * texture->width, mesh->uvs[i].y * texture->height).b / 255);
		Vector3 texture1((float)texture->getPixel(mesh->uvs[i + 1].x * texture->width, mesh->uvs[i + 1].y * texture->height).r / 255, (float)texture->getPixel(mesh->uvs[i + 1].x * texture->width, mesh->uvs[i + 1].y * texture->height).g / 255, (float)texture->getPixel(mesh->uvs[i + 1].x * texture->width, mesh->uvs[i + 1].y * texture->height).b / 255);
		Vector3 texture2((float)texture->getPixel(mesh->uvs[i + 2].x * texture->width, mesh->uvs[i + 2].y * texture->height).r / 255, (float)texture->getPixel(mesh->uvs[i + 2].x * texture->width, mesh->uvs[i + 2].y * texture->height).g / 255, (float)texture->getPixel(mesh->uvs[i + 2].x * texture->width, mesh->uvs[i + 2].y * texture->height).b / 255);

		//convert local coordinate to world coordinates
		Vector3 wPos_0 = (model_matrix * Vector4(p0.x, p0.y, p0.z, 1.0)).getVector3();
		Vector3 wPos_1 = (model_matrix * Vector4(p1.x, p1.y, p1.z, 1.0)).getVector3();
		Vector3 wPos_2 = (model_matrix * Vector4(p2.x, p2.y, p2.z, 1.0)).getVector3();

		Vector3 L0(light_pos - wPos_0); L0.normalize();
		Vector3 L1(light_pos - wPos_1); L1.normalize();
		Vector3 L2(light_pos - wPos_2); L2.normalize();

		Vector3 N0 = v0_normal.normalize();
		Vector3 N1 = v1_normal.normalize();
		Vector3 N2 = v2_normal.normalize();


		Vector3 V0(eye_pos - wPos_0); V0.normalize();
		Vector3 V1(eye_pos - wPos_1); V1.normalize();
		Vector3 V2(eye_pos - wPos_2); V2.normalize();

		float LdotN_0 = clamp(L0.dot(N0), 0, 1);
		float LdotN_1 = clamp(L1.dot(N1), 0, 1);
		float LdotN_2 = clamp(L2.dot(N2), 0, 1);

		Vector3 R0(-L0.x - 2 * (-L0.dot(N0)) * N0.x, -L0.y - 2 * (-L0.dot(N0)) * N0.y, -L0.z - 2 * (-L0.dot(N0)) * N0.z); R0.normalize();
		Vector3 R1(-L1.x - 2 * (-L1.dot(N1)) * N1.x, -L1.y - 2 * (-L1.dot(N1)) * N1.y, -L1.z - 2 * (-L1.dot(N1)) * N1.z); R1.normalize();
		Vector3 R2(-L2.x - 2 * (-L2.dot(N2)) * N2.x, -L2.y - 2 * (-L2.dot(N2)) * N2.y, -L2.z - 2 * (-L2.dot(N2)) * N2.z); R2.normalize();

		float meterial_shin = 30;

		float RdotV_0 = clamp(R0.dot(V0), 0, 1); RdotV_0 = pow(RdotV_0, 30);
		float RdotV_1 = clamp(R1.dot(V1), 0, 1); RdotV_1 = pow(RdotV_1, 30);
		float RdotV_2 = clamp(R2.dot(V2), 0, 1); RdotV_2 = pow(RdotV_2, 30);

		//project the vertex by the model view projection 
		Vector4 Position_0 = viewprojection * Vector4(wPos_0.x, wPos_0.y, wPos_0.z, 1.0);
		Vector4 Position_1 = viewprojection * Vector4(wPos_1.x, wPos_1.y, wPos_1.z, 1.0);
		Vector4 Position_2 = viewprojection * Vector4(wPos_2.x, wPos_2.y, wPos_2.z, 1.0);

		/*float dst_squared_0 = wPos_0.distance(light_pos);
		float dst_squared_1 = wPos_1.distance(light_pos);
		float dst_squared_2 = wPos_2.distance(light_pos);*/

		//dst_squared_0 = dst_squared_0 * dst_squared_0;
		//dst_squared_1 = dst_squared_1 * dst_squared_1;
		//dst_squared_2 = dst_squared_2 * dst_squared_2;

		Vector3 amb_0(light_amb.x * texture0.x, light_amb.y * texture0.y, light_amb.z * texture0.z);
		Vector3 amb_1(light_amb.x * texture1.x, light_amb.y * texture1.y, light_amb.z * texture1.z);
		Vector3 amb_2(light_amb.x * texture2.x, light_amb.y * texture2.y, light_amb.z * texture2.z);

		Vector3 dif_0(light_dif.x * LdotN_0 * texture0.x, light_dif.y * LdotN_0 * texture0.y, light_dif.z * LdotN_0 * texture0.z);
		Vector3 dif_1(light_dif.x * LdotN_1 * texture1.x, light_dif.y * LdotN_1 * texture1.y, light_dif.z * LdotN_1 * texture1.z);
		Vector3 dif_2(light_dif.x * LdotN_2 * texture2.x, light_dif.y * LdotN_2 * texture2.y, light_dif.z * LdotN_2 * texture2.z);

		Vector3 spc_0(light_dif.x * RdotV_0 * texture0.x, light_dif.y * RdotV_0 * texture0.y, light_dif.z * RdotV_0 * texture0.z);
		Vector3 spc_1(light_dif.x * RdotV_1 * texture1.x, light_dif.y * RdotV_1 * texture1.y, light_dif.z * RdotV_1 * texture1.z);
		Vector3 spc_2(light_dif.x * RdotV_2 * texture2.x, light_dif.y * RdotV_2 * texture2.y, light_dif.z * RdotV_2 * texture2.z);

		//compute color
		Vector3 color0 = amb_0 + dif_0 + spc_0;
		Vector3 color1 = amb_1 + dif_1 + spc_1;
		Vector3 color2 = amb_2 + dif_2 + spc_2;

		Color fragcolor0(color0.x * 255, color0.y * 255, color0.z * 255);
		Color fragcolor1(color1.x * 255, color1.y * 255, color1.z * 255);
		Color fragcolor2(color2.x * 255, color2.y * 255, color2.z * 255);

		int min_x = (std::min)((std::min)(v0.x, v1.x), v2.x);
		int max_x = (std::max)((std::max)(v0.x, v1.x), v2.x);
		int min_y = (std::min)((std::min)(v0.y, v1.y), v2.y);
		int max_y = (std::max)((std::max)(v0.y, v1.y), v2.y);


		Vector3 p;
	for (int x = min_x; x <= max_x; x++) {
		for (int y = min_y; y <= max_y; y++){
				//assuming p0,p1 and p2 are the vertices 2D
				p.set(x, y, 0);
				Vector3 bc = barycentricCoordinates(p, v0, v1, v2);
				p.z = v0.z * bc.x + v1.z * bc.y + v2.z * bc.z;

				//use weights to compute final color
				Color finalcolor = fragcolor0 * bc.x + fragcolor1 * bc.y + fragcolor2 * bc.z;

				// check if pixel uv is in coordinate range
				if (bc.x < 0 || bc.y < 0 || bc.z < 0)
					continue;
				// check if pixel is out of view
				if (p.z > 0 || p.x >= window_width || p.x < 0 || p.y >= window_height || p.y < 0)
					continue;
				// check if it occludes a pixel that is more in front
				if (p.z >= zbuffer.getPixel(p.x, p.y)) {
					zbuffer.setPixel(p.x, p.y, p.z);
					// scale coords and set pixel
					framebuffer.setPixel(p.x, p.y, finalcolor);
				}
			}
		}

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
