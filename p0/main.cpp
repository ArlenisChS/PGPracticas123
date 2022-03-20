#include <PGUPV.h>

using namespace PGUPV;

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

	// Vértices de los trángulos que forman la figura
	const std::vector<glm::vec2> vertices { 
		glm::vec2(-0.9, -0.9), glm::vec2(0.8, -0.9), glm::vec2(-0.9, 0.8),
		glm::vec2(0.9, 0.9), glm::vec2(-0.8, 0.9), glm::vec2(0.9, -0.8)
	};
	mesh->addVertices(vertices);

	// Especificar comando para dibujar los triángulos que se forman del vector anterior
	mesh->addDrawCommand(new PGUPV::DrawArrays(
		GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size())));

	model.addMesh(mesh);
}

void MyRender::setup() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Establecer variable de entrada del shader
	program.addAttributeLocation(0, "vPosition");

	// Cargar, compilar y usar shader "triangles"
	program.loadFiles("triangles");
	program.compile();
	program.use();

	// Método donde se define la figura que se desea dibujar.
	setupModel();
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT);

	// Renderizar modelo
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