

#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"
#include "GUI3.h"

using namespace PGUPV;

using glm::vec2;
using glm::ivec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

Programa de demostración de las distintas configuraciones que se le pueden aplicar 
a las texturas

*/


// Ruta que contienen las texturas. Puedes añadir aquí tus imágenes y reiniciar el programa 
#define IMGPATH "../recursos/imagenes/"

class MyRender : public Renderer {
public:
  MyRender()
    : anisotropyExtAvailable(isGLExtensionAvailable(
      "GL_EXT_texture_filter_anisotropic")) {};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
  void applyTextureParams();

private:
  std::vector<std::string> filenames;
  std::shared_ptr<GLMatrices> mats;
  std::shared_ptr<Program> program;
  Rect plane;
  std::vector<std::shared_ptr<Texture2D>> text; // Objetos textura
  bool anisotropyExtAvailable; // Indica si la extensión está disponible en la
  // máquina
  void buildGUI();

  std::shared_ptr<ListBoxWidget<>> textureSelector;
  std::shared_ptr<ListBoxWidget<GLenum>> wrapS, wrapT, filterMag, filterMin;
  std::shared_ptr<RGBAColorWidget> borderColor;
  std::shared_ptr<FloatSliderWidget> anisotropySlider;
  GLint offsetTextCoordLoc;
};

void MyRender::setup() {
  // Buscamos las texturas en el directorio definido
  filenames = listFiles(IMGPATH, false, std::vector < std::string > {"*.png", "*.jpg"});
  // Cargamos las texturas desde fichero
  for (std::size_t i = 0; i < filenames.size(); i++) {
    auto t = std::make_shared<Texture2D>();
    t->loadImage(filenames[i]);
    t->generateMipmap();
    text.push_back(t);
  }

  glClearColor(.7f, .7f, .7f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  /* Este shader se encarga de calcular la iluminación, teniendo
    en cuenta una textura */
  program = std::make_shared<Program>();
  program->addAttributeLocation(Mesh::VERTICES, "position");
  program->addAttributeLocation(Mesh::TEX_COORD0, "texCoord");

  mats = GLMatrices::build();
  program->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

  program->loadFiles("../ej3-2/ej3-2");
  program->compile();

  // Localización del uniform con la unidad de textura
  GLint texUnitLoc;
  texUnitLoc = program->getUniformLocation("texUnit");
  program->use();
  glUniform1i(texUnitLoc, 0);

  auto material = std::make_shared<Material>("textured");
  plane.getMesh(0).setMaterial(material);

  // Localización del uniform con el offset de las coordenadas de textura
  offsetTextCoordLoc = program->getUniformLocation("offsetTextureCoord");

  setCameraHandler(std::make_shared<OrbitCameraHandler>(1.5f));

  buildGUI();
  App::getInstance().getWindow().showGUI(true);

  // Aplica los parámetros a la textura activa y actualiza el GUI
  applyTextureParams();
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

  // Cuadrilátero texturado
  plane.render();

  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  uint t = MIN(w, h);
  glViewport(0, 0, t, t);
  mats->setMatrix(GLMatrices::PROJ_MATRIX,
    glm::perspective(glm::radians(60.0f), 1.0f, .1f, 100.0f));
  mats->loadIdentity(GLMatrices::MODEL_MATRIX);
}

void MyRender::applyTextureParams() {
  uint textBound = textureSelector->getSelected();
  // Aplicar la textura a la primera malla del objeto
  std::dynamic_pointer_cast<Material>(plane.getMesh(0).getMaterial())->setDiffuseTexture(text[textBound]);

  // Definir el color del borde
  glm::vec4 bc = borderColor->getColor();
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &bc.r);
  text[textBound]->setWrapS(wrapS->getSelectedUserData());
  text[textBound]->setWrapT(wrapT->getSelectedUserData());
  text[textBound]->setMagFilter(filterMag->getSelectedUserData());
  text[textBound]->setMinFilter(filterMin->getSelectedUserData());
  if (anisotropyExtAvailable) {
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropySlider->get());
  }
}

void MyRender::buildGUI() {

  auto panel = addPanel("Modos de texturado");
  panel->setPosition(500, 10);
  panel->setSize(290, 560);

  // Se encarga de invocar a la función para actualizar los parámetros de texturado
  auto updater = [this](const int &) { applyTextureParams(); };

  // Selección de la textura
  // Mostramos únicamente el nombre del fichero, no la ruta
  std::vector<std::string> names;
  for (auto name : filenames) {
    names.push_back(getFilenameFromPath(name));
  }
  textureSelector = std::make_shared < ListBoxWidget<> >("Textura", names);
  textureSelector->getValue().addListener(updater);
  textureSelector->setNumVisibleElements(4);
  panel->addWidget(textureSelector);

  // Multiplicador de las coordenadas de textura
  auto tcMult = std::make_shared<IVecSliderWidget<glm::ivec2>>("Mult. coord. text.", glm::ivec2(2, 2), 1, 10, program, "textureCoordMult");
  panel->addWidget(tcMult);

  auto textureOrigin = std::make_shared<ListBoxWidget<>>("Origen en:", std::vector<std::string> {"Esq. inf. izda", "Centro"});
  textureOrigin->getValue().addListener([this](int idx) {
    if (idx == 0) {
      glUniform2f(offsetTextCoordLoc, 0.0f, 0.0f);
    }
    else {
      glUniform2f(offsetTextCoordLoc, 0.5f, 0.5f);
    }
  });
  panel->addWidget(textureOrigin);

  // Modos de repetición de textura
  std::vector<std::string> wrapModesNames{
    "GL_REPEAT", "GL_MIRRORED_REPEAT", "GL_CLAMP_TO_EDGE", "GL_CLAMP_TO_BORDER" };
  std::vector<GLenum> wrapModesEnums{
    GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
  };
  // OpenGL 4.4 introdujo un nuevo tipo de repetición
  if (App::getInstance().getWindow().getOpenGLVersion().isGreaterEqual(GLVersion(4, 4))) {
    wrapModesNames.push_back("GL_MIRROR_CLAMP_TO_EDGE");
    wrapModesEnums.push_back(GL_MIRROR_CLAMP_TO_EDGE);
  }

  // Repetición en la dirección s
  wrapS = std::make_shared<ListBoxWidget<GLenum>>("Wrap S", wrapModesNames);
  wrapS->setUserData(wrapModesEnums);
  panel->addWidget(wrapS);
  wrapS->getValue().addListener(updater);

  // Repetición en la dirección s
  wrapT = std::make_shared<ListBoxWidget<GLenum>>("Wrap T", wrapModesNames);
  wrapT->setUserData(wrapModesEnums);
  panel->addWidget(wrapT);
  wrapT->getValue().addListener(updater);

  // Tipos de filtrado
  // Sólo NEAREST y LINEAR para aumento
  std::vector<std::string> filterModesNames{ "GL_NEAREST", "GL_LINEAR" };
  std::vector<GLenum> filterModesEnums{ GL_NEAREST, GL_LINEAR };

  // Filtro de aumento
  filterMag = std::make_shared<ListBoxWidget<GLenum>>("Mag filter", filterModesNames);
  filterMag->setUserData(filterModesEnums);
  panel->addWidget(filterMag);
  filterMag->getValue().addListener(updater);

  // Todos los filtros para reducción
  filterModesNames.push_back("GL_NEAREST_MIPMAP_NEAREST");
  filterModesNames.push_back("GL_NEAREST_MIPMAP_LINEAR");
  filterModesNames.push_back("GL_LINEAR_MIPMAP_NEAREST");
  filterModesNames.push_back("GL_LINEAR_MIPMAP_LINEAR");

  filterModesEnums.push_back(GL_NEAREST_MIPMAP_NEAREST);
  filterModesEnums.push_back(GL_NEAREST_MIPMAP_LINEAR);
  filterModesEnums.push_back(GL_LINEAR_MIPMAP_NEAREST);
  filterModesEnums.push_back(GL_LINEAR_MIPMAP_LINEAR);


  // Filtro de reducción
  filterMin = std::make_shared<ListBoxWidget<GLenum>>("Min filter", filterModesNames);
  filterMin->setUserData(filterModesEnums);
  panel->addWidget(filterMin);
  filterMin->getValue().addListener(updater);

  // Color del borde
  borderColor = std::make_shared<RGBAColorWidget>("Border color", glm::vec4(.0f, .0f, .0f, .0f));
  panel->addWidget(borderColor);
  borderColor->getValue().addListener([this](const glm::vec4 &) { applyTextureParams(); });

  if (anisotropyExtAvailable) {
    GLfloat max;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);

    anisotropySlider = std::make_shared<FloatSliderWidget>("Anisotropy", 1.0f, 1.0f, max);
    panel->addWidget(anisotropySlider);
    anisotropySlider->getValue().addListener([this](float ) { applyTextureParams(); });
  }
  else {
    panel->addWidget(std::make_shared<Label>("Anisotropy not available in this system"));
  }
}


int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.setInitWindowSize(800, 600);
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
    PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
