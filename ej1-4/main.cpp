#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;

/*

Ejemplo de dibujo indexado. Este programa dibuja un cubo en modo alámbrico
dibujando una línea (un segmento) para cada arista del cubo.

*/

class MyRender : public Renderer {
public:

  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
private:
  GLuint vao;
  GLuint vbos[3];
  /* Un programa es un conjunto de shaders que se pueden instalar en la GPU para calcular el
  aspecto final en pantalla de las primitivas dibujadas. Por ahora usaremos programas existentes.
  */
  Program program;
  /*
  Esta variable contiene las matrices modelo, vista y proyección que se utilizarán en el programa
  */
  std::shared_ptr<GLMatrices> mats;
};

/*
El método setup se ejecuta una vez, antes de empezar a dibujar. Se usa, por ejemplo, para establecer los
valores iniciales de dibujado (como, por ejemplo, el color de fondo), o para cargar modelos desde fichero.
*/
void MyRender::setup() {
  // Establece el color que se usará al borrar el buffer de color
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // Definición de los vértices y colores del modelo
  GLfloat vertices[][3] = {
    // Top face. Counterclockwise 
    { -0.5f, 0.5f, 0.5f },
    { 0.5f, 0.5f, 0.5f },
    { 0.5f, 0.5f, -0.5f },
    { -0.5f, 0.5f, -0.5f },

    // Bottom face. These points are below P0-P3
    { -0.5f, -0.5f, 0.5f },
    { 0.5f, -0.5f, 0.5f },
    { 0.5f, -0.5f, -0.5f },
    { -0.5f, -0.5f, -0.5f }
  };

  GLushort indices[] = {
    0, 4, 0, 3, 0, 1,
    5, 4, 5, 6, 5, 1,
    2, 3, 2, 6, 2, 1,
    7, 4, 7, 3, 7, 6
  };

  // Creamos un Vertex Array Object (VAO)
  glGenVertexArrays(1, &vao);
  // Lo vinculamos (a partir de ahora, todas las operaciones que trabajen con un VAO lo harán con este
  glBindVertexArray(vao);
  // Creamos dos buffer objects
  glGenBuffers(2, vbos);
  // Vinculamos el primer buffer object (BO) a GL_ARRAY_BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
  // Escribimos en el BO vinculado a GL_ARRAY_BUFFER el contenido del vector vertices
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Establecemos el formato de la información del atributo de vértice 0. En este caso, dicho
  // atributo está formado por 3 floats, sin normalizar, no hay espacio entre un atributo y el 
  // siguiente, y el primer elemento está al principio del BO vinculado.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  // Habilitar el atributo de vértice 0
  glEnableVertexAttribArray(0);

  // Para el color, establecemos el atributo genérico 2 al color negro
  glDisableVertexAttribArray(2);
  glVertexAttrib4f(2, 0.0, 0.0, 0.0, 1.0);
  
  // Repetir el proceso anterior para el segundo BO, donde almacenamos los índices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


  // Construímos el objeto GLMatrices
  mats = GLMatrices::build();

  // Enlazamos los atributos anteriores, con variables de entrada del shader de vértice
  // La localización debe coincidir con la usada más arriba
  program.addAttributeLocation(0, "position");
  program.addAttributeLocation(2, "vertcolor");
  // Conectamos el objeto mats con un índice de Uniform Buffer Object predeterminado 
  program.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  // Cargar los shaders que componen el programa (todos los shaders se llaman igual, con 
  // distintas extensiones
  program.loadFiles("../recursos/shaders/constantshading");
  // Compilar el programa
  program.compile();
}

/*
Esta función se ejecuta cada vez que haya que dibujar un nuevo frame. Por defecto,
la librería PGUPV está repintando continuamente la escena.
*/
void MyRender::render() {
  // Borramos el buffer de color
  glClear(GL_COLOR_BUFFER_BIT);

  mats->setMatrix(GLMatrices::VIEW_MATRIX,
    glm::lookAt(glm::vec3(1, 1, 2),
    glm::vec3(0, 0, 0),
    glm::vec3(0, 1, 0)));

  // Instalamos el programa ya compilado
  program.use();
  // Vinculamos el VAO que contiene los atributos a utilizar
  glBindVertexArray(vao);
    
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, 0);
}

/*
Esta función se ejecuta cada vez que el usuario cambie el tamaño de la ventana o al arrancar
el programa. Recibe por parámetro el nuevo tamaño de la ventana.
*/
void MyRender::reshape(uint w, uint h) {
  // Establecemos el viewport a toda la ventana
  glViewport(0, 0, w, h);
  if (h == 0) h = 1;
  float ar = (float)w / h;
  // Definimos una cámara perspectiva con la misma razón de aspecto que la ventana
  mats->setMatrix(GLMatrices::PROJ_MATRIX, glm::perspective(glm::radians(60.0f), ar, .1f, 10.0f));
}


int main(int argc, char *argv[]) {
  // Obtenemos una referencia al objeto App (sólo hay uno)
  App &myApp = App::getInstance();
  // Inicializamos la aplicación, pidiendo una ventana con: doble buffer, z-buffer y multisampling
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER | PGUPV::MULTISAMPLE);
  // Instalamos nuestra clase en la ventana
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  // Empezamos la ejecución de la aplicación (el método App::run sólo vuelve al cerrarse la ventana)
  return myApp.run();
}