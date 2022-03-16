
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "PGUPV.h"
#include "GUI3.h"
#include "videoDevice.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

Ejemplo básico de texturación: texturando un cuadrilátero con una textura de vídeo

*/

class MyRender : public Renderer {
public:
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
private:
  std::shared_ptr<GLMatrices> mats;
  Program ashader;
  GLint texUnitLoc;
  Rect plane;
  std::shared_ptr<TextureVideo> text;
  void buildGUI();
  void updateModelMatrix();
  std::shared_ptr < ListBoxWidget<bool> > sourcesList;
};

/*
Ajusta las proporciones del rectángulo (mediante un escalado no uniforme) para 
que coincidan con las de los fotogramas del vídeo.
*/
void MyRender::updateModelMatrix() {
  uint h = text->getHeight();
  uint w = text->getWidth();
  mats->loadIdentity(GLMatrices::MODEL_MATRIX);
  if (w > h)
    mats->scale(GLMatrices::MODEL_MATRIX, 1.0f, static_cast<float>(h) / w, 1.0f);
  else
    mats->scale(GLMatrices::MODEL_MATRIX, static_cast<float>(w) / h, 1.0f, 1.0f);
}

void MyRender::setup() {
  CHECK_GL();
  glClearColor(1.f, 1.f, 1.f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  /* Este shader se encarga de aplicar una textura a los objetos dibujados */
  // Posición de los vértices
  ashader.addAttributeLocation(Mesh::VERTICES, "position");
  // Coordenadas de textura
  ashader.addAttributeLocation(Mesh::TEX_COORD0, "texCoord");

  mats = GLMatrices::build();
  ashader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  ashader.loadFiles("../recursos/shaders/textureReplace");
  ashader.compile();

  // Localización del uniform con la unidad de textura
  texUnitLoc = ashader.getUniformLocation("texUnit");

  // Le decimos al shader qué unidad de textura utilizar
  ashader.use();
  glUniform1i(texUnitLoc, 0);

  buildGUI();

  App::getInstance().getWindow().showGUI();
  setCameraHandler(std::make_shared<OrbitCameraHandler>(2.0f));
}

void MyRender::render() {
  CHECK_GL();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

  // Dibujamos el plano texturado
  ashader.use();
  if (text) {
	  text->bind();
	  plane.render();
  }

  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
}

void MyRender::buildGUI() {
  auto panel = addPanel("Visor");
  panel->setPosition(550, 10);
  panel->setSize(240, 180);

  auto fc = std::make_shared<CheckBoxWidget>("Play", true);
  fc->getValue().addListener([this](const bool &play) {
    text->pause(!play);
  });
  panel->addWidget(fc);

  panel->addWidget(std::make_shared<Button>("Rebobinar", [this]() {
    text->rewind();
  }));

  // Listado de todas las cámaras encontradas en el sistema, y todos los ficheros
  // que se encuentren en ../recursos/videos/
  auto sources = media::VideoDevice::getAvailableCameras();
  // Este vector tendrá un booleano para cada entrada de 'sources', indicando si es un
  // un vídeo (true) o una cámara (false)
  std::vector<bool> sourcesIsVideo(sources.size(), false);
  auto videos = PGUPV::listFiles("../recursos/videos/", false);
  for (const auto &v : videos) {
	  sources.push_back(v);
	  sourcesIsVideo.push_back(true);
  }

  sourcesList = std::make_shared<ListBoxWidget<bool>>("Fuentes", sources);

  sourcesList->getValue().addListener([this, fc](const int &index) {
    if (sourcesList->getUserData(index))
      // Es un vídeo
      text = std::make_shared<TextureVideo>(sourcesList->getSelectedElement());
    else
      // Cámara
      text = std::make_shared<TextureVideo>(index);

    text->pause(!fc->get());

    updateModelMatrix();
  });
  sourcesList->setUserData(sourcesIsVideo);
  panel->addWidget(sourcesList);
  sourcesList->setSelected(0);
}


int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.setInitWindowSize(800, 600);
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
    PGUPV::MULTISAMPLE);
  CHECK_GL();
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
