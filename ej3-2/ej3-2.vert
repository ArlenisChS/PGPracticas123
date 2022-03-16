#version 420 core

$GLMatrices

// Multiplicador de la coordenada de textura (para convertir, por ejemplo,
// la coordenada (0, 1) en (0, textureCoordMult.y)
uniform ivec2 textureCoordMult;
uniform vec2 offsetTextureCoord;
in vec3 position;
in vec2 texCoord;

out vec2 texCoordFrag;

void main()
{
	texCoordFrag = textureCoordMult * (texCoord - offsetTextureCoord);
	gl_Position = modelviewprojMatrix *  vec4(position, 1.0) ;
}