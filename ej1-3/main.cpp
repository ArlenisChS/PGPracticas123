#include <glm/gtc/matrix_transform.hpp>

#include <PGUPV.h>

/*
Esta aplicación muestra los ejes coordenados en 3D
*/

using namespace PGUPV;

using glm::vec3;

class MyRender : public Renderer {
public:
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  // El objeto Axes contiene unos ejes predefinidos (eje X rojo, eje Y verde y
  // eje Z azul)
  Axes axes;
  Program program;
  // Esta variable contiene las matrices modelo, vista y proyección que se
  // utilizarán en el programa
  std::shared_ptr<GLMatrices> mats;
};

void MyRender::setup() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  /*
  Todos los modelos predefinidos (los que se encuentra en stockModels) y los que
  se cargan desde fichero definen los atributos estándar en índices predefinidos
  en la clase Mesh.
  Por ejemplo, la posición de los vértices están en el índice Mesh:VERTICES (0), 
  tal y como se define en Mesh.h.
  De esta forma todo el trabajo de definición de los VAO y los VBO lo realiza la
  librería, y nosotros sólo tenemos que decir el nombre de las variables de
  entrada del shader de vértice (atributos) que se corresponde con cada índice.
  */
  program.addAttributeLocation(Mesh::VERTICES, "position");
  program.addAttributeLocation(Mesh::COLORS, "vertcolor");

  // Construímos el objeto GLMatrices
  mats = GLMatrices::build();

  // Conectamos el objeto mats con un índice de Uniform Buffer Object
  // predeterminado
  program.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  program.loadFiles("../recursos/shaders/constantshading");
  program.compile();
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT);
  // Posiciona la cámara en el punto 1,1,2, mirando hacia el origen, y orientada
  // en vertical. Dado que la cámara no se mueve, se podría llevar la siguiente
  // línea a la función setup, pero la dejamos aquí porque lo normal es
  // actualizar la posición de la cámara al empezar a dibujar la escena.
  mats->setMatrix(GLMatrices::VIEW_MATRIX,
    glm::lookAt(vec3(1, 1, 2), vec3(0, 0, 0), vec3(0, 1, 0)));
  // Instalamos el programa
  program.use();
  // Dibujamos los ejes con el programa instalado
  axes.render();
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

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
