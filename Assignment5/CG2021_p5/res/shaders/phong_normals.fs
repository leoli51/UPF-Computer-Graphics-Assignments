
//this var comes from the vertex shader
varying vec2 v_coord;
varying vec3 v_wPos;

//the texture passed from the application
uniform mat4 model;
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

const vec3 normal_min = vec3(-1.0,-1.0,-1.0);
const vec3 normal_max = vec3(1.0,1.0,1.0);

void main()
{

	//read the pixel RGBA from the texture at the position v_coord
	vec4 material_color = texture2D( color_texture, v_coord );
	vec3 normal = texture2D(normal_texture, v_coord).xyz;
	
	// rotate the normals!!! use model matrix
	normal = mix(normal_min, normal_max, normal);
	normal = (model * vec4(normal, 0.0)).xyz;

    //calculate vector
	vec3 L = normalize(light_pos - v_wPos);
	vec3 N = normalize(normal);
	vec3 R = normalize(reflect(-L, N));
	vec3 V =  normalize(eye_pos - v_wPos);
	float LdotN = max(0.0, dot(L, N));
	float RdotV = max(0.0, dot(R, V));
	RdotV = pow(RdotV, mat_shin);

    float dst_squared = /* distance(eye_pos, v_wPos) +*/ distance(v_wPos, light_pos);
    dst_squared = dst_squared * dst_squared;

	vec3 amb = light_amb * material_color.xyz;
	vec3 dif = light_dif / dst_squared  * LdotN * material_color.xyz;

    vec3 Ks = material_color.xyz * material_color.w;
	vec3 spc = light_spc / dst_squared * RdotV * Ks;

	//compute color
	vec3 color = amb + dif + spc;

	//assign the color to the output
	gl_FragColor = vec4(color, 1.0);
}
