#include <PGUPV.h>
#include <GUI3.h>
#include <map>

using namespace PGUPV;

std::map<std::string, glm::vec4> loadMTL(std::string path,
	std::string file_name, glm::vec4 new_color) {
	std::stringstream ss;
	std::ifstream in_file(path + file_name);
	std::string line = "";
	std::string prefix = "";

	std::string current_key;
	std::map<std::string, glm::vec4> dict_material;

	//File open error check
	if (!in_file.is_open()) {
		throw "ERROR::MTLLOADER::Could not open file.";
	}

	if (new_color.x != 0 || new_color.y != 0
		|| new_color.z != 0 || new_color.w != 0) {
		while (std::getline(in_file, line)) {
			ss.clear();
			ss.str(line);
			ss >> prefix;

			if (prefix == "newmtl") {
				ss >> current_key;
				dict_material[current_key] = new_color;
			}
		}
		return dict_material;
	}

	//Read one line at a time
	while (std::getline(in_file, line)) {
		ss.clear();
		ss.str(line);
		ss >> prefix;

		if (prefix == "newmtl") {
			ss >> current_key;
		}
		else if (prefix == "Kd") {
			glm::vec4 temp_vec4;
			ss >> temp_vec4.x >> temp_vec4.y >> temp_vec4.z;
			temp_vec4.w = 1;
			dict_material[current_key] = temp_vec4;
		}
	}

	return dict_material;
}

std::shared_ptr<Model> loadOBJ(std::string path, std::string file_name,
	glm::vec4 new_color = glm::vec4(0, 0, 0, 0)) {

	std::vector<glm::vec3> vertex_positions;
	std::vector<glm::vec2> vertex_texcoords;
	std::vector<glm::vec3> vertex_normals;
	std::map<std::string, glm::vec4> vertex_materials;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> colors;

	std::stringstream ss;
	std::ifstream in_file(path + file_name);
	std::string line = "";
	std::string prefix = "";

	glm::vec3 temp_vec3;
	glm::vec2 temp_vec2;
	std::string temp_vertex_face;
	std::string current_color = "";

	//File open error check
	if (!in_file.is_open()) {
		throw "ERROR::OBJLOADER::Could not open file.";
	}

	//Read one line at a time
	while (std::getline(in_file, line)) {
		ss.clear();
		ss.str(line);
		ss >> prefix;

		if (prefix == "mtllib") {
			ss.ignore(1, ' ');
			std::string mtl_file;

			std::getline(ss, mtl_file);
			vertex_materials = loadMTL(path, mtl_file, new_color);
		}
		else if (prefix == "usemtl") {
			ss >> current_color;
		}
		else if (prefix == "v") {
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_positions.push_back(temp_vec3);
		}
		else if (prefix == "vt") {
			ss >> temp_vec2.x >> temp_vec2.y;
			vertex_texcoords.push_back(temp_vec2);
		}
		else if (prefix == "vn") {
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_normals.push_back(temp_vec3);
		}
		else if (prefix == "f") {
			while (ss >> temp_vertex_face) {
				std::stringstream temp_glint(temp_vertex_face);
				std::string segment;
				int counter = -1;

				while (std::getline(temp_glint, segment, '/')) {
					++counter;

					if (segment == "") continue;
					if (counter == 0) {
						colors.push_back(vertex_materials[current_color]);
						positions.push_back(vertex_positions[std::stoi(segment) - 1]);
					}
					else if (counter == 1)
						texcoords.push_back(vertex_texcoords[std::stoi(segment) - 1]);
					else if (counter == 2)
						normals.push_back(vertex_normals[std::stoi(segment) - 1]);
				}
			}
		}
	}

	auto mesh = std::make_shared<Mesh>();
	mesh->addVertices(positions);
	if (!normals.empty()) mesh->addNormals(normals);
	if (!texcoords.empty()) mesh->addTexCoord(texcoords[0].length(), texcoords);
	mesh->addColors(colors);
	mesh->addDrawCommand(new PGUPV::DrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(positions.size())));

	auto model = std::make_shared<Model>();
	model->addMesh(mesh);

	in_file.close();
	//Loaded success
	std::cout << "OBJ file loaded!" << "\n";
	return model;
}


class MyRender : public Renderer {
public:
	MyRender() = default;
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;
	void update(uint) override;

private:
	void buildModels();
	std::shared_ptr<GLMatrices> mats;
	Axes axes;
	std::vector<std::shared_ptr<Model>> models;

	void buildGUI();
	std::shared_ptr<IntSliderWidget> modelSelector;
};

/**
Construye tus modelos y añádelos al vector models (quita los ejemplos siguientes). Recuerda
que no puedes usar directamente las clases predefinidas (tienes que construir los meshes y los
models a base de vértices, colores, etc.)
*/
void MyRender::buildModels()
{
	auto tree1 = loadOBJ("../recursos/modelos/trees/tree1/", "tree1.obj");
	models.push_back(tree1);
	auto tree2 = loadOBJ("../recursos/modelos/trees/tree2/", "tree2.obj");
	models.push_back(tree2);
	auto tree3 = loadOBJ("../recursos/modelos/trees/tree3/", "tree3.obj");
	models.push_back(tree3);
}

void MyRender::setup() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Habilitamos el z-buffer
	glEnable(GL_DEPTH_TEST);
	// Habilitamos el back face culling. ¡Cuidado! Si al dibujar un objeto hay 
	// caras que desaparecen o el modelo se ve raro, puede ser que estés 
	// definiendo los vértices del polígono del revés (en sentido horario)
	// Si ese es el caso, invierte el orden de los vértices.
	// Puedes activar/desactivar el back face culling en cualquier aplicación 
	// PGUPV pulsando las teclas CTRL+B
	glEnable(GL_CULL_FACE);

	mats = PGUPV::GLMatrices::build();

	// Activamos un shader que dibuja cada vértice con su atributo color
	ConstantIllumProgram::use();

	buildModels();

	// Establecemos una cámara que nos permite explorar el objeto desde cualquier
	// punto
	setCameraHandler(std::make_shared<OrbitCameraHandler>());

	// Construimos la interfaz de usuario
	buildGUI();
}

void MyRender::render() {
	// Borramos el buffer de color y el zbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Le pedimos a la cámara que nos de la matriz de la vista, que codifica la
	// posición y orientación actuales de la cámara.
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

	// Dibujamos los ejes
	axes.render();

	// Dibujamos los objetos
	if (!models.empty()) {
		auto current_model = models[modelSelector->get()];
		mats->pushMatrix(GLMatrices::MODEL_MATRIX);
		mats->scale(GLMatrices::MODEL_MATRIX, glm::vec3(1.0f / current_model->maxDimension()));
		current_model->render();
		mats->popMatrix(GLMatrices::MODEL_MATRIX);
	}

	// Si la siguiente llamada falla, quiere decir que OpenGL se encuentra en
	// estado erróneo porque alguna de las operaciones que ha ejecutado
	// recientemente (después de la última llamada a CHECK_GL) ha tenido algún
	// problema. Revisa tu código.
	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
}

// Este método se ejecuta una vez por frame, antes de llamada a render. Recibe el 
// número de milisegundos que han pasado desde la última vez que se llamó, y se suele
// usar para hacer animaciones o comprobar el estado de los dispositivos de entrada
void MyRender::update(uint) {
	// Si el usuario ha pulsado el espacio, ponemos la cámara en su posición inicial
	if (App::isKeyUp(PGUPV::KeyCode::Space)) {
		getCameraHandler()->resetView();
	}
}

/**
En éste método construimos los widgets que definen la interfaz de usuario. En esta
práctica no tienes que modificar esta función.
*/
void MyRender::buildGUI() {
	auto panel = addPanel("Modelos");
	modelSelector = std::make_shared<IntSliderWidget>("Model", 0, 0, static_cast<int>(models.size()-1));

	if (models.size() <= 1) {
		panel->addWidget(std::make_shared<Label>("Introduce algún modelo más en el vector models..."));
	}
	else {
		panel->addWidget(modelSelector);
	}
	App::getInstance().getWindow().showGUI();
}


int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
		PGUPV::MULTISAMPLE);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
