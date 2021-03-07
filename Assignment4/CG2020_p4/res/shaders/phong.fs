// constant values 
uniform vec3 light_pos;
uniform vec3 light_dif;
uniform vec3 light_spc;
uniform vec3 light_amb;
uniform vec3 material_dif;
uniform vec3 material_spc;
uniform vec3 material_amb;
uniform float material_shin;
uniform vec3 eye_pos;

//this var comes from the vertex shader
//they are baricentric interpolated by pixel according to the distance to every vertex
varying vec3 v_wPos;
varying vec3 v_wNormal;

//here create uniforms for all the data we need here

void main()
{
	//calculate vector
	vec3 L = normalize(light_pos - v_wPos);
	vec3 N = normalize(v_wNormal);
	vec3 R = normalize(reflect(-L, N));
	vec3 V =  normalize(eye_pos - v_wPos);
	float LdotN = max(0.0, dot(L, N));
	float RdotV = max(0.0, dot(R, V));
	RdotV = pow(RdotV, material_shin);

    float dst_squared = distance(eye_pos, v_wPos) + distance(v_wPos, light_pos);
    dst_squared = dst_squared * dst_squared;

	vec3 amb = light_amb * material_amb;
	vec3 dif = light_dif / dst_squared  * LdotN * material_dif;
	vec3 spc = light_spc / dst_squared * RdotV * material_spc;

	//compute color
	vec3 color = amb + dif + spc;

	//set the ouput color por the pixel
	gl_FragColor = vec4(color, 1.0);
}
