#version 420

$GLMatrices

in vec3 position;

void main()
{
	gl_Position = modelviewprojMatrix * vec4(position,1.0);
}
