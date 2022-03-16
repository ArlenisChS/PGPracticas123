
// Para utilizar la librería PGUPV tienes que incluir la siguiente cabecera:
#include <PGUPV.h>

/*
Todas las clases de la librería PGUPV están definidas dentro del namespace
PGUPV. Para usar cualquier clase en C++, debes especificar el espacio de nombres
donde se encuentra. Para ello, tienes tres opciones:

1. Calificar con el espacio de nombres cada aparición de la clase en el código:
// Declarando el objeto program
PGUPV::Program program;

2. Declarar que el uso de una clase hace referencia a un namespace dado:

using PGUPV::Program;
// Declarando el objeto program
Program program;

3. Importar todas las clases de un namespace:

using namespace PGUPV;
// Declarando el objeto program
Program program;

En los ejemplos de la asignatura usaremos indistintamente la opción 2 o 3.
*/

using PGUPV::App;
using PGUPV::Renderer;
using PGUPV::Program;
using PGUPV::GLMatrices;

/*
El primer paso para usar la librería PGUPV es crear una clase que herede de
PGUPV::Renderer
Deberás implementar, como mínimo, los métodos setup, render y reshape.
*/
class MyRender : public Renderer {
public:
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  GLuint vao;
  GLuint vbos[2];
  /* Un programa es un conjunto de shaders que se pueden instalar en la GPU para
  calcular el aspecto final en pantalla de las primitivas dibujadas. Por ahora
  usaremos programas existentes.
  */
  Program program;
};

/*
El método setup se ejecuta una vez, antes de empezar a dibujar. Se usa, por
ejemplo, para establecer los valores iniciales de dibujado (como, por ejemplo,
el color de fondo), o para cargar modelos desde fichero.
*/
void MyRender::setup() {
  // Establece el color que se usará al borrar el buffer de color
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // Definición de los vértices y colores del modelo
  GLfloat vertices[] = { -0.5, -0.5, 0.0, 0.5, -0.5, 0.0, 0.0, 0.5, 0.0 };
  GLfloat colores[] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

  // Creamos un Vertex Array Object (VAO)
  glGenVertexArrays(1, &vao);
  // Lo vinculamos (a partir de ahora, todas las operaciones que trabajen con un
  // VAO lo harán con este
  glBindVertexArray(vao);
  // Creamos dos buffer objects
  glGenBuffers(2, vbos);
  // Vinculamos el primer buffer object (BO) a GL_ARRAY_BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
  // Escribimos en el BO vinculado a GL_ARRAY_BUFFER el contenido del vector
  // vertices
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Establecemos el formato de la información del atributo de vértice 0. En
  // este caso, dicho atributo está formado por 3 floats, sin normalizar, no hay
  // espacio entre un atributo y el siguiente, y el primer elemento está al
  // principio del BO vinculado.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  // Habilitar el atributo de vértice 0
  glEnableVertexAttribArray(0);

  // Repetir el proceso anterior para el segundo BO, donde almacenamos los
  // colores
  glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colores), colores, GL_STATIC_DRAW);
  // Fíjate que ahora usamos el atributo de vértice 2 (el uno se queda sin usar)
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  glEnableVertexAttribArray(2);

  // Enlazamos los atributos anteriores, con variables de entrada del shader de
  // vértice. La localización debe coincidir con la usada más arriba
  program.addAttributeLocation(0, "position");
  program.addAttributeLocation(2, "vertcolor");
  // Cargar los shaders que componen el programa (todos los shaders se llaman
  // igual, con distintas extensiones
  program.loadFiles("../ej1-1/passthrough");
  // Compilar el programa
  program.compile();
}

/*
Esta función se ejecuta cada vez que haya que dibujar un nuevo frame. Por
defecto, la librería PGUPV está repintando continuamente la escena.
*/
void MyRender::render() {
  // Borramos el buffer de color
  glClear(GL_COLOR_BUFFER_BIT);
  // Instalamos el programa ya compilado
  program.use();
  // Vinculamos el VAO que contiene los atributos a utilizar
  glBindVertexArray(vao);
  // Dibujamos un triángulo con los primeros tres vértices del VAO vinculado
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

/*
Esta función se ejecuta cada vez que el usuario cambie el tamaño de la ventana o
al arrancar el programa. Recibe por parámetro el nuevo tamaño de la ventana.
*/
void MyRender::reshape(uint w, uint h) {
  // Establecemos el viewport a toda la ventana
  glViewport(0, 0, w, h);
}

int main(int argc, char *argv[]) {
  // Obtenemos una referencia al objeto App (sólo hay uno)
  App &myApp = App::getInstance();
  // Inicializamos la aplicación, pidiendo una ventana con: doble buffer,
  // z-buffer y multisampling
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
    PGUPV::MULTISAMPLE);
  // Instalamos nuestra clase en la ventana
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  // Empezamos la ejecución de la aplicación (el método App::run sólo vuelve al
  // cerrarse la ventana)
  return myApp.run();
}