

#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;
using glm::vec3;

/*

Ejemplo de uso de arrays de texturas 2D para mostrar un GIF animado

*/

class MyRender : public Renderer {
public:
  MyRender()
    : texture(new Texture2DArray(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)),
    frame(0) {};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
  void update(uint) override;

private:
  std::shared_ptr<GLMatrices> mats;
  Axes axes;
  Program cshader, ashader;
  GLint texUnitLoc, frameUnitLoc;
  Rect plane;
  std::shared_ptr<Texture2DArray> texture;
  uint frame;
};

/*
Este método se ejecuta repetidamente. Cada vez que se ejecuta, recibe por
parámetro el número de ms transcurridos desde la última llamada.
*/
void MyRender::update(uint ms) {
  static uint total = 0;

  total += ms;
  // Si han pasado 250 ms desde la última actualización, cambiamos de frame
  if (total > 250) {
    total = 0;
    frame++;
    if (frame == texture->getDepth())
      frame = 0;
    glUniform1i(frameUnitLoc, frame);
  }
}

void MyRender::setup() {
  glClearColor(1.f, 1.f, 1.f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  mats = GLMatrices::build();

  /* Este shader se encarga dibujar los objetos con las coordenadas de textura
  indicadas. Además, tiene un uniform llamado "frame", que indica qué nivel del
  array de texturas utilizar. */
  ashader.addAttributeLocation(Mesh::VERTICES, "position");
  ashader.addAttributeLocation(Mesh::TEX_COORD0, "texCoord");

  ashader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

  ashader.loadFiles("../ej3-3/texture2DArray");
  ashader.compile();

  // Localización de los uniform (unidad de textura y capa del array a mostrar)
  texUnitLoc = ashader.getUniformLocation("texUnit");
  frameUnitLoc = ashader.getUniformLocation("frame");

  ashader.use();
  glUniform1i(texUnitLoc, 0);
  glUniform1i(frameUnitLoc, 0);

  // Cargamos la nueva textura desde un fichero
  texture->loadImage("../recursos/imagenes/spid1.gif");
  // Ajustamos el rectángulo a la razón de aspecto de la textura
  mats->scale(GLMatrices::MODEL_MATRIX,
    (float)texture->getWidth() / texture->getHeight(), 1.0f, 1.0f);

  mats->setMatrix(GLMatrices::VIEW_MATRIX,
    glm::lookAt(vec3(0, 0, 2), vec3(0.0f), vec3(0, 1, 0)));

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  texture->bind();
  plane.render();

  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  if (h == 0)
    h = 1;
  float ar = (float)w / h;
  mats->setMatrix(GLMatrices::PROJ_MATRIX,
    glm::perspective(glm::radians(60.0f), ar, .1f, 100.0f));
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
    PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
