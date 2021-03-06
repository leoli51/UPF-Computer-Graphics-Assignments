// in Gouraud shading the color are interpolated through the pixels
// in Phong shading the  Normals are interpolated through the pixels

//global variables from the CPU
uniform mat4 model;
uniform mat4 viewprojection;

// light parameters
uniform vec3 light_pos;
uniform vec4 light_color;
uniform float light_intensity;
uniform int p;
uniform float reflected_light_coefficient;

// camera parameters
uniform vec3 camera_pos;

//vars to pass to the pixel shader
varying vec3 v_color; // interpolated color to pass to the fragment shader

//here create uniforms for all the data we need here

void main()
{	
	//convert local coordinate to world coordinates
	vec3 wPos = (model * vec4( gl_Vertex.xyz, 1.0)).xyz;
	//convert local normal to world coordinates
	vec3 wNormal = (model * vec4( gl_Normal.xyz, 0.0)).xyz;

	//in GOURAUD compute the color here and pass it to the pixel shader
	//...
    vec3 view_dir = normalize(camera_pos - wPos);
    vec3 incident_dir = normalize(wPos - light_pos);
    vec3 reflected_light_dir = reflect(wNormal, incident_dir);
    float dst = distance(camera_pos, wPos) + distance(light_pos, wPos);
    float reflected_light_amount = reflected_light_coefficient * (light_intensity / (dst * dst)) * pow(max(0, dot(view_dir, reflected_light_dir), p);
    v_color = reflected_light_amount * light_color;

	//project the vertex by the model view projection 
	gl_Position = viewprojection * vec4(wPos,1.0); //output of the vertex shader
}