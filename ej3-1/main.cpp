#include "PGUPV.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

Ejemplo básico de texturación: texturando un cuadrilátero

*/

class MyRender : public Renderer {
public:
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;

private:
	std::shared_ptr<GLMatrices> mats;
	std::shared_ptr<Texture2D> texture;
	Rect plane;
};

void MyRender::setup() {
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	mats = GLMatrices::build();

	// Cambiando las coordenadas del objeto Rect original. Por defecto,
	// las coordenadas de textura de dicho objeto están entre 0 y 1 (mira
	// el constructor de Rect en stockModels.cpp
	glm::vec2 tc[] = {
	  glm::vec2(0.0f, 0.0f), glm::vec2(2.0f, 0.0f),
	  glm::vec2(2.0f, 2.0f), glm::vec2(0.0f, 2.0f) };

	// Añadir las coordenadas de textura a la primera malla (y única)
	// malla del plano.
	plane.getMesh(0).addTexCoord(0, tc, 4);

	// Cargamos la nueva textura desde un fichero
	texture = std::make_shared<Texture2D>();
	texture->loadImage("../recursos/imagenes/checker.png");
	setCameraHandler(std::make_shared<OrbitCameraHandler>(2.0f));
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

	// Vinculamos la textura a la unidad de textura 0
	texture->bind(GL_TEXTURE0);

	// Instalamos un shader que dibuja los modelos aplicándoles la textura activa
	// Por defecto usa la unidad de textura 0, pero puedes cambiarla con la llamada
	// TextureReplaceProgram::setTextureUnit(1), para usar la textura en GL_TEXTURE1
	TextureReplaceProgram::use();

	// Dibujamos el plano texturado
	plane.render();

	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
}

int main(int argc, char *argv[]) {
	App &myApp = App::getInstance();
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
		PGUPV::MULTISAMPLE);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
