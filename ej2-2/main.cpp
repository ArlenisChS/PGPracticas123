#include <glm/gtc/matrix_transform.hpp>

#include <sstream>
#include <iomanip>

#include "PGUPV.h"
#include "GUI3.h"

using namespace PGUPV;

using glm::vec3;
using glm::mat4;
using std::vector;

/* 
Aplicación de demostración del uso de los distintos parámetros de blending.
Podrás cambiar en vivo los parámetros de blending para ver el resultado.
*/

class MyRender : public Renderer {
public:
  MyRender() :
    cuadro(4.0f, 4.0f),
    offset(0.0f, 0.0f, 0.0f)
  {};

  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
  void update(uint ms) override;
  bool mouse_move(const MouseMotionEvent &e) override;
private:
  void buildGUI();
  std::shared_ptr<GLMatrices> mats;
  Program shader;
  Rect cuadro;
  GLint colorLoc;
  glm::vec3 offset;
  uint windowWidth, windowHeight;

  std::shared_ptr<CheckBoxWidget> enableBlending;
  std::shared_ptr<RGBAColorWidget> srcColor;
  std::shared_ptr<ListBoxWidget<GLenum>> blendEq, blendSrcFactor, blendDestFactor;
  std::shared_ptr<Label> currentColor;
};


void MyRender::buildGUI() {
  // Creamos un panel nuevo donde introducir nuestros controles, llamado Configuracion
  auto panel = addPanel("Blending");
  // Tamaño del panel y posición del panel
  panel->setSize(260, 520);
  panel->setPosition(530, 10);

  // Creamos un checkbox para activar o desactivar el blending
  enableBlending = std::make_shared<CheckBoxWidget>("Activar blending", false);
  // Añadimos el checkbox en el panel
  panel->addWidget(enableBlending);

  // Creamos un widget para pedir un color RGBA
  srcColor = std::make_shared<RGBAColorWidget>("Color fuente", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
  // y lo añadimos al panel
  panel->addWidget(srcColor);

  // Selector de la ecuación de blending (sumar, restar, etc)
  // Creamos un vector de cadenas con las opciones que queremos mostrar en una lista
  std::vector<std::string> blendEqNames;
  std::vector<GLenum> blendEqEnums;

#define BE(F) blendEqNames.push_back(#F); blendEqEnums.push_back(F);
  BE(GL_FUNC_ADD)
    BE(GL_FUNC_SUBTRACT)
    BE(GL_FUNC_REVERSE_SUBTRACT)
    BE(GL_MIN)
    BE(GL_MAX)
#undef BE

    // Creamos un listbox con los elementos anteriores
    blendEq = std::make_shared<ListBoxWidget<GLenum>>("Blend equation", blendEqNames);
  blendEq->setUserData(blendEqEnums);
  panel->addWidget(blendEq);

  // Ahora añadimos un listener para que cada vez que el usuario seleccione un elemento de la lista,
  // se cambie en el estado de OpenGL:
  blendEq->getValue().addListener([this](int) {
    glBlendEquation(blendEq->getSelectedUserData());
  });

  // Selector de los factores fuente y destino (GL_ZERO, GL_SRC_ALPHA...)
  // Creamos un vector de cadenas con las opciones que queremos mostrar en una lista
  std::vector<std::string> blendFuncNames;
  std::vector<GLenum> blendFuncEnums;
#define BF(F) blendFuncNames.push_back(#F); blendFuncEnums.push_back(F);
  BF(GL_ZERO)
    BF(GL_ONE)
    BF(GL_SRC_COLOR)
    BF(GL_ONE_MINUS_SRC_COLOR)
    BF(GL_DST_COLOR)
    BF(GL_ONE_MINUS_DST_COLOR)
    BF(GL_SRC_ALPHA)
    BF(GL_ONE_MINUS_SRC_ALPHA)
    BF(GL_DST_ALPHA)
    BF(GL_ONE_MINUS_DST_ALPHA)
    BF(GL_CONSTANT_COLOR)
    BF(GL_ONE_MINUS_CONSTANT_COLOR)
    BF(GL_CONSTANT_ALPHA)
    BF(GL_ONE_MINUS_CONSTANT_ALPHA)
    BF(GL_SRC_ALPHA_SATURATE)
#undef BF

  // Creamos un listbox para el factor fuente
  // El valor inicial del factor fuente es GL_ONE
  blendSrcFactor = std::make_shared<ListBoxWidget<GLenum>>("Src Factor", blendFuncNames, 1);
  blendSrcFactor->setUserData(blendFuncEnums);
  blendSrcFactor->setNumVisibleElements(8);
  panel->addWidget(blendSrcFactor);

  // Creamos un listbox para el factor destino
  // El valor inicial del factor destino es GL_ZERO
  blendDestFactor = std::make_shared<ListBoxWidget<GLenum>>("Dest Factor", blendFuncNames, 0);
  blendDestFactor->setUserData(blendFuncEnums);
  blendDestFactor->setNumVisibleElements(8);
  panel->addWidget(blendDestFactor);

  // Ahora añadimos un listener para que cada vez que el usuario seleccione un elemento de la lista,
  // se cambie en el estado de OpenGL. Usamos el mismo listener para los factores fuente y destino
  auto setBlendFactors = [this](int) {
    glBlendFunc(blendSrcFactor->getSelectedUserData(), blendDestFactor->getSelectedUserData());
  };
  blendSrcFactor->getValue().addListener(setBlendFactors);
  blendDestFactor->getValue().addListener(setBlendFactors);

  // Selector del color constante
  auto constantColor = std::make_shared<RGBAColorWidget>("Constant blend color", glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  panel->addWidget(constantColor);
  constantColor->getValue().addListener([](const glm::vec4 &c) { glBlendColor(c.r, c.g, c.b, c.a); });

  currentColor = std::make_shared<Label>("Color puntero");
  panel->addWidget(currentColor);

  App::getInstance().getWindow().showGUI(true);
}

void MyRender::setup() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  shader.addAttributeLocation(Mesh::VERTICES, "position");
  shader.addAttributeLocation(Mesh::COLORS, "vertcolor");
  mats = GLMatrices::build();
  shader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  // Este shader dibuja el modelo con el color especificado en la variable uniform 'color'
  shader.loadFiles("../recursos/shaders/constantshadinguniform");
  shader.compile();
  colorLoc = shader.getUniformLocation("primitive_color");
  // Construimos el interfaz de usuario
  buildGUI();
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT);
  uint min = MIN(windowWidth, windowHeight);
  glViewport(0, 0, min, min);
  shader.use();

  // Cuadros de fondo
  mats->setMatrix(GLMatrices::MODEL_MATRIX, glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 2.0f, 0.0f)));
  glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
  cuadro.render();
  mats->setMatrix(GLMatrices::MODEL_MATRIX, glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 0.0f)));
  glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
  cuadro.render();
  mats->setMatrix(GLMatrices::MODEL_MATRIX, glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, -2.0f, 0.0f)));
  glUniform4f(colorLoc, 0.0f, 0.0f, 1.0f, 1.0f);
  cuadro.render();
  mats->setMatrix(GLMatrices::MODEL_MATRIX, glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -2.0f, 0.0f)));
  glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
  cuadro.render();

  if (enableBlending->get()) {
    glEnable(GL_BLEND);
    // Las llamadas a glBlendEquation y glBlendFunc se producen en las callbacks de los widgets
    // correspondientes
  }
  // Cuadro en primer plano en movimiento
  mats->setMatrix(GLMatrices::MODEL_MATRIX, glm::translate(glm::mat4(1.0f), offset));
  mats->scale(GLMatrices::MODEL_MATRIX, 0.5, 0.5, 1.0);
  glm::vec4 color = srcColor->getColor();
  glUniform4fv(colorLoc, 1, &color.r);
  cuadro.render();

  if (enableBlending->get()) {
    glDisable(GL_BLEND);
  }
  glViewport(0, 0, windowWidth, windowHeight);
}

void MyRender::reshape(uint w, uint h) {
  if (h == 0) h = 1;
  windowWidth = w;
  windowHeight = h;
  mats->setMatrix(GLMatrices::PROJ_MATRIX, glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f));
}


void MyRender::update(uint ms) {
  // velocidad: 1 unidad por segundo
  static glm::vec3 delta = glm::vec3(1.0, 1.0, 0.0);

  offset += delta * (ms / 1000.0f);
  if (offset.x > 4.0f) {
    delta = -delta;
    offset.x = offset.y = 4.0f;
  }
  else if (offset.x < -4.0f) {
    delta = -delta;
    offset.x = offset.y = -4.0f;
  }
}

// Esta función se llama cada vez que se mueve el ratón
bool MyRender::mouse_move(const MouseMotionEvent &e) {
  unsigned char c[4];
  // Leer el color del píxel que se encuentra bajo el puntero del ratón
  glReadPixels(e.x, windowHeight - e.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, c);
  std::ostringstream s;
  s << "Puntero ratón: " << (unsigned int)c[0] << "," << (unsigned int)c[1] << "," << (unsigned int)c[2] << "," << (unsigned int)c[3];
  // Actualizar el texto de la etiqueta
  currentColor->setText(s.str());
  // Devolvemos false para dejar que otros manejadores tengan acceso al ratón
  return false;
}


int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.setInitWindowSize(800, 600);
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::RGBA);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}

