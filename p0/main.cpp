#include <PGUPV.h>

using namespace PGUPV;

/* 
Rellena las funciones setup y render tal y como se explica en el enunciado de la práctica.
¡Cuidado! NO uses las llamadas de OpenGL directamente (glGenVertexArrays, glBindBuffer, etc.).
Usa las clases Model y Mesh de PGUPV.
*/

class MyRender : public Renderer {
public:
	void setup(void);
	void render(void);
	void reshape(uint w, uint h);

private:
	Model model;
	Program program;

	void setupModel();
};

void MyRender::setupModel() {
	auto mesh = std::make_shared<Mesh>();

	const std::vector<glm::vec2> vertices { 
		glm::vec2(-0.9, -0.9), glm::vec2(0.8, -0.9), glm::vec2(-0.9, 0.8),
		glm::vec2(0.9, 0.9), glm::vec2(-0.8, 0.9), glm::vec2(0.9, -0.8)
	};

	mesh->addVertices(vertices);
	mesh->addDrawCommand(new PGUPV::DrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size())));

	model.addMesh(mesh);
}

void MyRender::setup() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	program.addAttributeLocation(0, "vPosition");
	program.loadFiles("triangles");
	program.compile();
	program.use();

	setupModel();
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT);
	model.render();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
}

int main(int argc, char *argv[]) {
	App &myApp = App::getInstance();
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}