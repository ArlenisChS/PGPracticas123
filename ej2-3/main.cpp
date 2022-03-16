#include <glm/gtc/matrix_transform.hpp>
#include "PGUPV.h"

using namespace PGUPV;

/*
Demostración de la depedencia del resultado del blending del orden en el que se
dibujan los objetos.
*/

#define OPACIDAD 0.7

class MyRender : public Renderer {
public:
  MyRender() :
    cuboRojo(0.5, 0.5, 0.5, glm::vec4(1.0, 0.0, 0.0, OPACIDAD)),
    cuboVerde(0.5, 0.5, 0.5, glm::vec4(0.0, 1.0, 0.0, OPACIDAD)),
    cuboAzul(0.5, 0.5, 0.5, glm::vec4(0.0, 0.0, 1.0, OPACIDAD))
  {};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
private:
  void dibujaColumna(void);
  Box cuboRojo, cuboVerde, cuboAzul;
  std::shared_ptr<GLMatrices> mats;
  Program shader;
};

void MyRender::setup() {
  glClearColor(0.6f, 0.6f, 0.9f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  // Activamos el back face culling para no dibujar las caras traseras
  glEnable(GL_CULL_FACE);
  shader.addAttributeLocation(Mesh::VERTICES, "position");
  shader.addAttributeLocation(Mesh::COLORS, "vertcolor");
  mats = GLMatrices::build();
  shader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  shader.loadFiles("../recursos/shaders/constantshading");
  shader.compile();

  // Dejamos activo el shader
  shader.use();

  // Modo de blending:
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  setCameraHandler(std::make_shared<OrbitCameraHandler>());
}

// Dibuja tres cubos en columna. Primero el rojo, luego el verde y luego el azul
void MyRender::dibujaColumna() {
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, 0.0f, 0.0f, 1.0f);
  cuboRojo.render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, 0.0f, 0.0f, 0.0f);
  cuboVerde.render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, 0.0f, 0.0f, -1.0f);
  cuboAzul.render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

  // Dibujamos primero los objetos opacos
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, -1.0f, 0.0f, 0.0f);
  dibujaColumna();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, 1.0f, 0.0f, 0.0f);
  dibujaColumna();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  // Desactivar la escritura al Z-buffer
  glDepthMask(GL_FALSE);
  // Ahora dibujamos los objetos translúcidos, en este orden: rojo, verde, azul
  // Ten en cuenta que el resultado no será el esperado, dependiendo del punto de vista
  glEnable(GL_BLEND);
  dibujaColumna();
  glDisable(GL_BLEND);
  // Activar la escritura al Z-buffer
  glDepthMask(GL_TRUE);
}

void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  // El manejador de cámara define una cámara perspectiva con la misma razón de aspecto que la ventana
  mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
  mats->loadIdentity(GLMatrices::MODEL_MATRIX);
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}