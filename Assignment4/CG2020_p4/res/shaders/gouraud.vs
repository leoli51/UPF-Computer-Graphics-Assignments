//global variables from the CPU
uniform mat4 model;
uniform mat4 viewprojection;
uniform vec3 light_pos;
uniform vec3 light_dif;
uniform vec3 light_spc;
uniform vec3 light_amb;
uniform vec3 material_dif;
uniform vec3 material_spc;
uniform vec3 material_amb;
uniform float material_shin;
uniform vec3 eye_pos;

//vars to pass to the pixel shader
varying vec3 v_wPos;
varying vec3 v_wNormal;
varying vec3 color;

//here create uniforms for all the data we need here

void main()
{	
	//convert local coordinate to world coordinates
	vec3 wPos = (model * vec4( gl_Vertex.xyz, 1.0)).xyz;
	//convert local normal to world coordinates
	vec3 wNormal = (model * vec4( gl_Normal.xyz, 0.0)).xyz;

	//pass them to the pixel shader interpolated
	v_wPos = wPos;
	v_wNormal = wNormal;

	//calculate vector
	vec3 L = normalize(light_pos - wPos);
	vec3 N = normalize(wNormal);
	vec3 R = reflect(L, N);
	vec3 V =  normalize(eye_pos - wPos);
	float LdotN = dot(L, N);
	LdotN = clamp(1, 0, 1);
	float RdotV = dot(R, V);
	//RdotV = clamp(1, 0, 1);
	RdotV = pow(RdotV, material_shin);

	//in GOURAUD compute the color here and pass it to the pixel shader
	vec3 amb = light_amb * material_amb;
	vec3 dif = light_dif * LdotN * material_dif;
	vec3 spc = light_spc * RdotV * material_spc;

	//compute color
	color = amb + dif + spc;


	//project the vertex by the model view projection 
	gl_Position = viewprojection * vec4(wPos,1.0); //output of the vertex shader
}