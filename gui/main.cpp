#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

/*
Esta aplicación muestra cómo crear interfaces gráficas de usuario con PGUPV

La forma de construir una interfaz gráfica de usuario con PGUPV es creando
uno o más paneles en la ventana, introduciendo los controles necesarios en
ellos.

Hay dos formas principales de trabajar con los controles: mediante una referencia
a ellos, para leer y escribir su contenido (por ejemplo, un control de tipo
FloatSliderWidget contiene un float, que el usuario puede modificar mediante una
interfaz en pantalla). A través de una referencia a dicho control podemos acceder
a su contenido, tanto para escribir en él (set) como para leer de él (get).
Por ejemplo:

std::shared_ptr<FloatSliderWidget> valor;

[...]

float f = valor->get();
f = f/2.0f;
valor->set(f);

cada vez que se ejecutaran las tres líneas de código anteriores, se dividiría por
2 el valor almacenado en el control (el control en pantalla se actualizaría
automáticamente, mostrando el nuevo valor).

La segunda forma de trabajar con los controles es mediante listeners (también conocidos
como manejadores de eventos o callbacks). Un listener es un objeto función
(https://en.wikipedia.org/wiki/Function_object) que se ejecutará cada vez que el
valor del control cambie. Dentro de ese objeto función se encuentra el código que
queremos ejecutar (en este caso puede que ni siquiera necesitemos una referencia
al control a nivel de clase). Siguiendo con el ejemplo anterior:

// Creamos el control
auto valor = std::make_shared<FloatSliderWidget>("Fondo", 0.5f, 0.0f, 1.0f);
valor->getValue().addListener([](float f) { glClearColor(f, f, f, 1.0f); });
panel->addWidget(valor);

En el ejemplo anterior, hemos instalado una lambda que se ejecutará cada vez que
se modifique el valor del slider. En este caso lo que hace es cambiar el color de
borrado de la pantalla a un valor de gris. El valor inicial del slider es 0.5, y
se puede mover entre 0.0 y 1.0 (ver los parámetros en el constructor).


Siempre que quieras crear una interfaz de usuario, tienes que incluir la siguiente
cabecera. Abre el fichero para ver qué tipos de controles tienes disponibles.

*/

#include "GUI3.h"


using namespace PGUPV;

using glm::vec3;

class MyRender : public Renderer {
public:
  MyRender() : cube(1.0f, glm::vec4(.7, .7, .7, 1.0)) {};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  Axes axes;
  WireBox cube;
  Program program;
  std::shared_ptr<GLMatrices> mats;

  // Definimos una función donde creamos la interfaz (se podría hacer dentro de setup
  // pero queda más limpio hacerlo aparte).
  void buildGUI();
  // Define aquí los widget que quieras consultar desde la función render
  // Mostrar/ocultar los ejes
  std::shared_ptr<CheckBoxWidget> showAxes;
  // Tamaño del cubo
  std::shared_ptr<FloatSliderWidget> cubeSize;
  // Rotación del cubo
  std::shared_ptr<RotationWidget> cubeRotation;
};

void MyRender::setup() {
  program.addAttributeLocation(Mesh::VERTICES, "position");
  program.addAttributeLocation(Mesh::COLORS, "vertcolor");

  // Construímos el objeto GLMatrices
  mats = GLMatrices::build();

  // Conectamos el objeto mats con un índice de Uniform Buffer Object
  // predeterminado
  program.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  program.loadFiles("../recursos/shaders/constantshading");
  program.compile();

  // Construimos la interfaz:
  buildGUI();

  // Por defecto la interfaz no se muestra (el usuario puede pulsar Ctrl+G para mostrarla)
  // Podemos hacer que se muestre con:
  App::getInstance().getWindow().showGUI(true);

  program.use();
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT);
  mats->setMatrix(GLMatrices::VIEW_MATRIX,
    glm::lookAt(vec3(1, 1, 2), vec3(0, 0, 0), vec3(0, 1, 0)));

  // Para comprobar el contenido de un widget, tenemos que usar la función ->get()
  if (showAxes->get())
    axes.render();

  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->setMatrix(GLMatrices::MODEL_MATRIX, cubeRotation->get());
  mats->scale(GLMatrices::MODEL_MATRIX, glm::vec3(cubeSize->get()));
  cube.render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);
}

void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  if (h == 0)
    h = 1;
  float ar = (float)w / h;
  // Definimos una cámara perspectiva con la misma razón de aspecto que la
  // ventana
  mats->setMatrix(GLMatrices::PROJ_MATRIX,
    glm::perspective(glm::radians(60.0f), ar, .1f, 10.0f));
}

void MyRender::buildGUI() {
  // El primer paso es crear un Panel. Un Panel es el contenedor de los widgets
  auto panel = addPanel("Ejemplo de GUI");
  // Podemos definir su tamaño y posición
  panel->setSize(340, 280); // Ancho, alto, en píxeles
  panel->setPosition(15, 15); // Respecto la esquina superior izquierda

  // Añadiendo una etiqueta
  panel->addWidget(std::make_shared<Label>("Una etiqueta..."));

  // Un checkbox permite al usuario mostrar u ocultar los ejes. Necesitamos guardar
  // la referencia a nivel de clase (showAxes es un miembro de la clase)
  showAxes = std::make_shared<CheckBoxWidget>("Mostrar ejes", true); // Etiqueta y valor inicial
  // Siempre hay que añadir el widget al panel 
  panel->addWidget(showAxes);

  // RGBAColorWidget permite al usuario definir un color RGBA
  // Como no necesitamos el valor de esta variable fuera de esta función, la declaramos aquí:
  auto backgroundColor = std::make_shared<RGBAColorWidget>("Color de fondo");
  // Ahora queremos establecer el color de borrado de OpenGL cada vez que el usuario cambie el 
  // color del widget. Para ello establecemos un listener (una función) que se ejecutará cada vez 
  // que el valor cambie de valor. Usamos una lambda para definir la función.
  backgroundColor->getValue().addListener([](glm::vec4 color) {
    glClearColor(color.r, color.g, color.b, color.a); });
  // No te olvides añadirlo al panel
  panel->addWidget(backgroundColor);

  // Un separador
  panel->addWidget(std::make_shared<Separator>());

  // Este FloatSliderWidget define el factor de escala del cubo
  cubeSize = std::make_shared<FloatSliderWidget>("Tamaño", 1.0f, 0.1f, 2.0f);
  panel->addWidget(cubeSize);

  // Un RotationWidget permite definir una rotación
  cubeRotation = std::make_shared<RotationWidget>("Rotación");
  panel->addWidget(cubeRotation);

  // Un botón para resetear el tamaño y rotación del cubo
  panel->addWidget(std::make_shared<Button>("Reset",
    [this]() { // Esta lambda captura el puntero this...
      cubeSize->set(1.0f);              // para poder acceder a las variables miembro
      cubeRotation->set(glm::mat4(1.0f));
  }));
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.setInitWindowSize(800, 600);
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
