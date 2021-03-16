//global variables from the CPU
uniform mat4 model;
uniform mat4 viewprojection;

//vars to pass to the pixel shader
varying vec2 v_coord;
varying vec3 v_wPos;
varying vec3 v_wNormal;

void main()
{	
    // pass position and normal to fragment shader
	vec3 wPos = (model * gl_Vertex).xyz;
    vec3 wNormal = (model * vec4( gl_Normal.xyz, 0.0)).xyz;

    v_wPos = wPos;
	v_wNormal = wNormal;

	//get the texture coordinates (per vertex) and pass them to the pixel shader
	v_coord = gl_MultiTexCoord0.xy;

	//project the vertex by the model view projection 
	gl_Position = viewprojection * vec4(wPos,1.0); //output of the vertex shader
}