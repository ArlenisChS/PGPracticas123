#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;

using glm::vec3;
using glm::mat4;
using std::vector;

/*

Ejemplo de uso de las cámaras interactivas y las pilas de matrices de transformación. 

*/

class MyRender : public Renderer {
public:
	void setup(void);
	void render(void);
	void reshape(uint w, uint h);
private:
	std::shared_ptr<GLMatrices> mats;
	Axes axes;
	Program shader;
	WireBox box;
};

void MyRender::setup() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	shader.addAttributeLocation(Mesh::VERTICES, "position");
	shader.addAttributeLocation(Mesh::COLORS, "vertcolor");
    mats = GLMatrices::build();
	shader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
	shader.loadFiles("../recursos/shaders/constantshading");
	shader.compile();
	setCameraHandler(std::make_shared<OrbitCameraHandler>());
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Matriz de la vista: la calcula la clase OrbitCameraHandler, a partir de las interacciones
	// del usuario con el ratón.
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());
	shader.use();
	axes.render();

	// Ejemplo de tres modelos definidos con respecto a un sistema de coordenadas común
	// El modelo utilizado es una caja centrada en el origen y alineada a los ejes de
	// tamaño 1x1x1

	/* Para dibujar el primer modelo, acumulamos en la matriz del modelo la siguiente
	transformación:
	
	1. Un escalado de 0.2, 0.2, 0.2 
	*/
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->scale(GLMatrices::MODEL_MATRIX, 0.2f, 0.2f, 0.2f);
	box.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	/* Para dibujar el segundo modelo, acumulamos en la matriz del modelo las siguientes
	transformaciones:
	
	1. Una rotación de 45º alrededor del eje Y
	2. Una traslación de 0.7, 0.5, 0.5 

	(fíjate en el orden, primero se aplica la rotación y luego la traslación!!!
	*/
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->translate(GLMatrices::MODEL_MATRIX, 0.7f, 0.5f, 0.5f);
	mats->rotate(GLMatrices::MODEL_MATRIX, glm::radians(45.f), 0.0f, 1.0f, 0.0f);
	box.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	/* Para dibujar el segundo modelo, se aplican, en este orden, las siguientes transformaciones:

	1. Un escalado de 0.2, 0.8 y 0.1
	2. Una traslación de 0, 0.4, 1.2

	*/
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->translate(GLMatrices::MODEL_MATRIX, 0.0f, 0.4f, 1.2f);
	mats->scale(GLMatrices::MODEL_MATRIX, 0.2f, 0.8f, 0.1f);
	box.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
	mats->loadIdentity(GLMatrices::MODEL_MATRIX);
}


int main(int argc, char *argv[]) {
	App &myApp = App::getInstance();
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}