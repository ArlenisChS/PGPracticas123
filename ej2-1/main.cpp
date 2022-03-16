#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*
En este ejemplo se muestra el uso del stencil buffer para recortar el dibujo de
una escena a un área determinada de la ventana.
*/

class MyRender : public Renderer {
public:
  MyRender() : teapotSpin(0.0f), lightPosLoc(0) {}
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
  void update(uint ms) override;

private:
  std::shared_ptr<GLMatrices> mats;
  Program ashader, nshader;  // Dos shaders
  std::shared_ptr<Scene> tetera, bisonte; // Modelos de una tetera y un bisonte
  float teapotSpin;          // Ángulo de rotación actual de la tetera
  vec4 lightPos;             // Posición de la luz en coordenadas del mundo
  GLuint lightPosLoc; // Localización del uniform que contiene la posición de la
  // luz
};

void MyRender::setup() {
  // Posición de la luz en la escena (espacio del mundo)
  lightPos = glm::vec4(2.0f, 5.0f, 5.0f, 1.0f);

  glClearColor(.1f, .1f, .1f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  // Cargando los modelos desde fichero
  tetera = FileLoader::load("../recursos/modelos/teapot.3ds");
  // Asignamos a todas las mallas de la tetera el material cobre
  auto copper = PGUPV::getMaterial(PredefinedMaterial::COPPER);
  tetera->processMeshes([copper](Mesh &m) { m.setMaterial(copper); });

  bisonte = FileLoader::load("../recursos/modelos/Wuson.ms3d");
  // Asignamos a todas las mallas del bisonte el material oro
  auto gold = PGUPV::getMaterial(PredefinedMaterial::GOLD);
  bisonte->processMeshes([gold](Mesh &m) { m.setMaterial(gold); });

  mats = GLMatrices::build();

  // El shader ambdiff necesita la posición y la normal de cada vértice
  ashader.addAttributeLocation(Mesh::VERTICES, "position");
  ashader.addAttributeLocation(Mesh::NORMALS, "normal");
  ashader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  // Insertamos la definición del material
  ashader.replaceString("$" + UBOMaterial::blockName, UBOMaterial::definition);
  ashader.loadFiles("../recursos/shaders/ambdiff");
  ashader.compile();
  // Localización del uniform que contiene la posición de la luz
  lightPosLoc = ashader.getUniformLocation("lightpos");

  // Este shader hace el trabajo mínimo para dibujar los modelos: dibuja todos
  // los fragmentos a blanco
  nshader.addAttributeLocation(Mesh::VERTICES, "position");
  nshader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  nshader.loadFiles("../recursos/shaders/touchpixels");
  nshader.compile();

  ashader.use();
  // Como la cámara está fija, la establecemos aquí
  mats->setMatrix(GLMatrices::VIEW_MATRIX,
    glm::lookAt(vec3(0.0f, 0.8f, 0.8f), vec3(0.0f, 0.25f, 0.0f),
      vec3(0.0f, 1.0f, 0.0f)));
  // Como la posición de la luz no cambia, ni la posición de la cámara tampoco,
  // sus coordenadas en el espacio de la cámara tampoco cambian.
  vec4 lp = mats->getMatrix(GLMatrices::VIEW_MATRIX) * lightPos;
  glUniform4f(lightPosLoc, lp.x, lp.y, lp.z, lp.w);
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Shader para dibujar los objetos
  ashader.use();

  // Activar el test de stencil
  glEnable(GL_STENCIL_TEST);

  // El test de stencil pasa si el contador asociado la posición del fragmento
  // está a 1
  glStencilFunc(GL_EQUAL, 0x1, 0x1);
  // En cualquier caso, no tocaremos el valor del stencil
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

  // Tetera que gira sobre sí misma
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->rotate(GLMatrices::MODEL_MATRIX, teapotSpin, vec3(0.0f, 1.0f, 0.0f));
  mats->scale(GLMatrices::MODEL_MATRIX, vec3(1.0f / tetera->maxDimension()));
  tetera->render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  // Ahora el test pasa cuando el valor del contador asociado a la posición del
  // fragmento no es 1
  glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);

  // Bisonte
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->rotate(GLMatrices::MODEL_MATRIX, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
  mats->scale(GLMatrices::MODEL_MATRIX, vec3(1.0f / bisonte->maxDimension()));
  bisonte->render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  glDisable(GL_STENCIL_TEST);
  CHECK_GL();
}

// Cada vez que el tamaño de la ventana cambie tenemos que actualizar el stencil
// buffer
void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  if (h == 0)
    h = 1;
  float ar = (float)w / h;

  // Preparando el stencil
  // Para dibujar en el stencil más fácilmente, asignamos una cámara ortográfica
  // de ancho 2 unidades
  mats->setMatrix(GLMatrices::PROJ_MATRIX,
    glm::ortho(-1.0f, 1.0f, -1.0f / ar, 1.0f / ar, 0.0f, 10.0f));
  // Situamos la cámara en (0, 0, 1), mirando hacia el origen
  // Guardamos la matriz de la vista actual
  mats->pushMatrix(GLMatrices::VIEW_MATRIX);
  mats->setMatrix(GLMatrices::VIEW_MATRIX,
    glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f)));
  // Borrar el stencil buffer
  glClear(GL_STENCIL_BUFFER_BIT);
  // Esta condición del stencil siempre pasa
  glStencilFunc(GL_ALWAYS, 0x1, 0x1);
  // En cualquier caso, siempre que toquemos un pixel, reemplazaremos el valor
  // de su stencil por el valor indicado
  glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

  // No necesitamos...
  glDisable(GL_DEPTH_TEST);                // z-buffer
  glColorMask(false, false, false, false); // escribir en el buffer de color

  // Activar el test del stencil
  glEnable(GL_STENCIL_TEST);
  // Instalar el shader que únicamente dibuja todos los fragmentos generados a
  // blanco
  nshader.use();

  // Definimos un rectángulo que cruza la mitad de la ventana de arriba a abajo
  Rect rectangulo(0.5f, 3.0f);
  // Al dibujarlo, pondremos el contador del stencil asociado a los fragmentos
  // de este rectángulo a 1
  rectangulo.render();

  // Desabilitar el test de stencil, activar la escritura al buffer de color, y
  // activar el z-buffer
  glDisable(GL_STENCIL_TEST);
  glColorMask(true, true, true, true);
  glEnable(GL_DEPTH_TEST);

  // Recuperamos la matriz de la vista guardada anteriormente
  mats->popMatrix(GLMatrices::VIEW_MATRIX);
  // Establecer la cámara perspectiva que se usará para dibujar la escena
  mats->setMatrix(GLMatrices::PROJ_MATRIX,
    glm::perspective(glm::radians(60.0f), ar, .1f, 100.0f));
}

// Radianes por segundo a los que gira la tetera
#define SPINSPEED glm::radians(180.0f)

void MyRender::update(uint ms) {
  teapotSpin += SPINSPEED*ms / 1000.0f;
  if (teapotSpin > TWOPIf) teapotSpin -= TWOPIf;
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
    PGUPV::STENCIL_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
