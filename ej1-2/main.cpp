#include <PGUPV.h>

using namespace PGUPV;

/*
Esta es la aplicación mínima que se puede ejecutar con PGUPV.
Simplemente borra la ventana a un color determinado.
*/

class MyRender : public Renderer {
public:
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
};

void MyRender::setup() { glClearColor(0.1f, 0.1f, 0.9f, 1.0f); }

void MyRender::render() { glClear(GL_COLOR_BUFFER_BIT); }

void MyRender::reshape(uint w, uint h) { glViewport(0, 0, w, h); }

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}