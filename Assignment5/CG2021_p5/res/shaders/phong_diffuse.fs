
//this var comes from the vertex shader
varying vec2 v_coord;
varying vec3 v_wPos;
varying vec3 v_wNormal;

//the texture passed from the application
uniform sampler2D color_texture;
uniform sampler2D normal_texture;

uniform vec3 light_pos;
uniform vec3 light_dif;
uniform vec3 light_spc;
uniform vec3 light_amb;
uniform float mat_shin;
uniform vec3 mat_dif;
uniform vec3 mat_spc;
uniform vec3 mat_amb;
uniform vec3 eye_pos;

void main()
{
	//read the pixel RGBA from the texture at the position v_coord
	vec4 material_color = texture2D( color_texture, v_coord );
    //calculate vector
	vec3 L = normalize(light_pos - v_wPos);
	vec3 N = normalize(v_wNormal);
	vec3 R = normalize(reflect(-L, N));
	vec3 V =  normalize(eye_pos - v_wPos);
	float LdotN = max(0.0, dot(L, N));
	float RdotV = max(0.0, dot(R, V));
	RdotV = pow(RdotV, mat_shin);

    float dst_squared = /* distance(eye_pos, v_wPos) +*/ distance(v_wPos, light_pos);
    dst_squared = dst_squared * dst_squared;

	vec3 amb = light_amb * material_color.xyz;
	vec3 dif = light_dif / dst_squared  * LdotN * material_color.xyz;
	vec3 spc = light_spc / dst_squared * RdotV * material_color.xyz;

	//compute color
	vec3 color = amb + dif + spc;

	//assign the color to the output
	gl_FragColor = vec4(color, 1.0);
}
