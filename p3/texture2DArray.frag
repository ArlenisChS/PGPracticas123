#version 420 core

uniform	sampler2DArray texUnit;


uniform int frame;

in vec2 texCoordFrag;
out vec4 fragColor;

void main()
{
fragColor = texture(texUnit, vec3(texCoordFrag, float(frame)));
}
