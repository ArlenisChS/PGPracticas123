#version 420

// Dos atributos de entrada:
in vec4 position;
in vec4 vertcolor;

/* Un valor de salida (el color final del vértice, que será interpolado entre los fragmentos 
generados al rasterizar la primitiva.
*/
out vec4 fragcolor;

void main()
{
	// Simplemente copiamos el color de entrada
	fragcolor = vertcolor;
	// Es obligatorio escribir en gl_Position la coordenada en clip space del vértice
	gl_Position = position;
}
