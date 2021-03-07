//this var comes from the vertex shader
//they are baricentric interpolated by pixel according to the distance to every vertex
varying vec3 color;

//here create uniforms for all the data we need here

void main()
{
	//set the ouput color por the pixel
	gl_FragColor = vec4( color, 1.0 );
}
