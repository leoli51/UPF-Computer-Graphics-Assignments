//global variables from the CPU
uniform mat4 model;
uniform mat4 viewprojection;

//here create uniforms for all the data we need here

void main()
{	
	//convert local coordinate to world coordinates
	vec3 wPos = (model * vec4( gl_Vertex.xyz, 1.0)).xyz;

	//project the vertex by the model view projection 
	gl_Position = viewprojection * vec4(wPos,1.0); //output of the vertex shader
}