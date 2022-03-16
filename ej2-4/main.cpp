
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "matrixStack.h"
#include "utils.h"

using std::cout;
using std::endl;
using PGUPV::to_string;

/* 

Este programa te ayudará a familiarizarte con el uso de la clase MatrixStack y 
con la biblioteca glm.

En utils.h tienes funciones para convertir variables de tipo glm::vec3, glm::mat4, 
etc. a cadena
*/


int main() {
	PGUPV::MatrixStack m;

	cout << "Matriz identidad: " << endl;
	m.loadIdentity();
	cout << to_string(m.getMatrix()) << endl;

	cout << "Matriz que traslada los vértices 10 unidades en las X positivas: " << endl;
	m.translate(10.0f, 0.0f, 0.0f);
	cout << to_string(m.getMatrix()) << endl;

	glm::vec4 p = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	cout << "p= " << to_string(p) << endl;

	cout << "M*p = " << to_string(m.getMatrix() * p) << endl;

	cout << "pushMatrix()" << endl;
	m.pushMatrix();
	m.rotate(glm::radians(90.0f), 0.0f, 1.0f, 0.0f);

	cout << "Ahora M = T(10, 0, 0)*R(PI/2, 0, 1, 0)" << endl;
	cout << "M=" << endl << to_string(m.getMatrix()) << endl;

	p=glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	cout << "p= " << to_string(p) << endl;

	cout << "M*p = " << to_string(m.getMatrix() * p) << endl;

	cout << "popMatrix()" << endl;
	m.popMatrix();
	cout << "M=" << endl << to_string(m.getMatrix()) << endl;

	return 0;
}
