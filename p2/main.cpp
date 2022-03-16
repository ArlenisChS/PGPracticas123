
#include "PGUPV.h"
#include <deque>

using namespace PGUPV;

#define MIN_DISTANCE 20
#define MAX_DISTANCE 40
#define VELOCITY -0.05f
#define ROAD_WIDHT_x2 1.f
#define FAR 200.f
#define NEAR 0.f
#define COLOR glm::vec4(0, 0, 1, 0.3)

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
	if (!texcoords.empty()) mesh->addTexCoord(0, texcoords);
	mesh->addColors(colors);
	mesh->addDrawCommand(new PGUPV::DrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(positions.size())));

	auto model = std::make_shared<Model>();
	model->addMesh(mesh);

	in_file.close();
	//Loaded success
	std::cout << "OBJ file loaded!" << "\n";
	return model;
}


int random(int a, int b) {
	return a + (std::rand() % (b - a));
}


class GameObject {
public:
	GameObject() {};
	GameObject(std::shared_ptr<Model> model) : _model(model) {};

	void render(std::shared_ptr<GLMatrices> mats);

	void setModel(std::shared_ptr<Model> model) {
		_model = model;
	};

	void rotate(float x, float y, float z) {
		angle_x += x; angle_y += y; angle_z += z;
	};
	void rotateX(float x) {
		angle_x = x;
	};
	void rotateY(float y) {
		angle_y = y;
	};
	void rotateZ(float z) {
		angle_z = z;
	};

	void translate(float x, float y, float z) {
		current_x += x; current_y += y; current_z += z;
	};
	void translateX(float x) {
		current_x = x;
	};
	void translateY(float y) {
		current_y = y;
	};
	void translateZ(float z) {
		current_z = z;
	};

	void scaleTo(float size) {
		this->size = size;
	};

	float distanceTo(glm::vec3 point);
	glm::vec3 getPosition() {
		return glm::vec3(current_x, current_y, current_z);
	};

	void setActiveRender(bool render_state) {
		active_render = render_state;
	}
	bool getActiveRender() {
		return active_render;
	}

private:
	std::shared_ptr<Model> _model;

	float current_x = 0;
	float current_y = 0;
	float current_z = 0;

	float angle_x = 0;
	float angle_y = 0;
	float angle_z = 0;

	float size = 0;

	bool active_render = true;
};

void GameObject::render(std::shared_ptr<GLMatrices> mats) {
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);

	mats->translate(GLMatrices::MODEL_MATRIX,
		current_x, current_y, current_z);

	mats->rotate(GLMatrices::MODEL_MATRIX, angle_x, glm::vec3{ 1.0f, 0.0f, 0.0f });
	mats->rotate(GLMatrices::MODEL_MATRIX, angle_y, glm::vec3{ 0.0f, 1.0f, 0.0f });
	mats->rotate(GLMatrices::MODEL_MATRIX, angle_z, glm::vec3{ 0.0f, 0.0f, 1.0f });

	if (size > 0) {
		mats->scale(GLMatrices::MODEL_MATRIX, glm::vec3(size / _model->maxDimension()));
	}

	_model->render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);
}

float GameObject::distanceTo(glm::vec3 point) {
	return glm::sqrt(glm::pow(current_x - point.x, 2)
		+ glm::pow(current_y - point.y, 2)
		+ glm::pow(current_z - point.z, 2));
}


class MyRender : public Renderer {
public:
	MyRender() {};
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;
	void update(uint ms) override;

private:
	std::shared_ptr<GLMatrices> mats;
	Axes axes;

	float current_left_distance = 0;
	float current_right_distance = 0;

	std::shared_ptr<GameObject> windshield;
	std::vector<std::shared_ptr<GameObject>> avilable_models;
	std::deque<std::shared_ptr<GameObject>> left_road;
	std::deque<std::shared_ptr<GameObject>> right_road;

	void setup_models();
	void render_models();
	void update_models(uint ms);
};

void MyRender::setup_models() {
	auto model1 = loadOBJ("../recursos/modelos/trees/tree1/", "tree1.obj");
	auto model2 = loadOBJ("../recursos/modelos/trees/tree2/", "tree2.obj");
	auto model3 = loadOBJ("../recursos/modelos/trees/tree3/", "tree3.obj");
	auto windshield_model = loadOBJ("../recursos/modelos/windshield/", 
		"windshield.obj", COLOR);
	
	windshield = std::make_shared<GameObject>(windshield_model);
	windshield->scaleTo(0.8);
	windshield->translateY(0.10);

	auto tree1 = std::make_shared<GameObject>(model1);
	avilable_models.push_back(tree1);
	tree1->scaleTo(1);

	auto tree2 = std::make_shared<GameObject>(model1);
	avilable_models.push_back(tree2);
	tree2->scaleTo(1);
	tree2->rotateY(glm::radians(90.0f));

	auto tree3 = std::make_shared<GameObject>(model1);
	avilable_models.push_back(tree3);
	tree3->scaleTo(1.2);
	tree3->rotateY(glm::radians(180.0f));

	auto tree4 = std::make_shared<GameObject>(model1);
	avilable_models.push_back(tree4);
	tree4->scaleTo(0.8);
	tree4->rotateY(glm::radians(270.0f));

	auto tree5 = std::make_shared<GameObject>(model1);
	avilable_models.push_back(tree5);
	tree5->scaleTo(1.1);
	tree5->rotateY(glm::radians(45.0f));


	auto tree6 = std::make_shared<GameObject>(model2);
	avilable_models.push_back(tree6);
	tree6->scaleTo(1);

	auto tree7 = std::make_shared<GameObject>(model2);
	avilable_models.push_back(tree7);
	tree7->scaleTo(1);
	tree7->rotateY(glm::radians(90.0f));

	auto tree8 = std::make_shared<GameObject>(model2);
	avilable_models.push_back(tree8);
	tree8->scaleTo(1.2);
	tree8->rotateY(glm::radians(180.0f));

	auto tree9 = std::make_shared<GameObject>(model2);
	avilable_models.push_back(tree9);
	tree9->scaleTo(0.8);
	tree9->rotateY(glm::radians(270.0f));

	auto tree10 = std::make_shared<GameObject>(model2);
	avilable_models.push_back(tree10);
	tree10->scaleTo(1.1);
	tree10->rotateY(glm::radians(45.0f));

	auto tree11 = std::make_shared<GameObject>(model3);
	avilable_models.push_back(tree11);
	tree11->scaleTo(1);

	auto tree12 = std::make_shared<GameObject>(model3);
	avilable_models.push_back(tree12);
	tree12->scaleTo(1.5);

	auto tree13 = std::make_shared<GameObject>(model3);
	avilable_models.push_back(tree13);
	tree13->scaleTo(2);

	auto tree14 = std::make_shared<GameObject>(model3);
	avilable_models.push_back(tree14);
	tree14->scaleTo(1.8);

	auto tree15 = std::make_shared<GameObject>(model1);
	avilable_models.push_back(tree15);
	tree15->scaleTo(2);

	auto tree16 = std::make_shared<GameObject>(model1);
	avilable_models.push_back(tree16);
	tree16->scaleTo(1.8);
	tree16->rotateY(glm::radians(90.0f));

	auto tree17 = std::make_shared<GameObject>(model1);
	avilable_models.push_back(tree17);
	tree17->scaleTo(2.1);
	tree17->rotateY(glm::radians(180.0f));

	auto tree18 = std::make_shared<GameObject>(model2);
	avilable_models.push_back(tree18);
	tree18->scaleTo(2);
	tree18->rotateY(glm::radians(180.0f));

	auto tree19 = std::make_shared<GameObject>(model2);
	avilable_models.push_back(tree19);
	tree19->scaleTo(2.1);
	tree19->rotateY(glm::radians(270.0f));

	auto tree20 = std::make_shared<GameObject>(model2);
	avilable_models.push_back(tree20);
	tree20->scaleTo(1.8);
	tree20->rotateY(glm::radians(900.0f));
}

void MyRender::setup() {
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_DEPTH_TEST);

	mats = GLMatrices::build();
	setup_models();

	mats->setMatrix(GLMatrices::VIEW_MATRIX,
		glm::lookAt(glm::vec3(0.f, 0.2f, -5.f), glm::vec3(0.f, 0.2f, 0.f),
			glm::vec3(0.0f, 1.0f, 0.0f)));

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MyRender::render_models() {
	for (auto& elem : left_road) {
		elem->render(mats);
	}

	for (auto& elem : right_road) {
		elem->render(mats);
	}

	if (!avilable_models.empty()) {
		if (left_road.empty() || 
			left_road.back()->distanceTo(glm::vec3(-ROAD_WIDHT_x2, 0, FAR)) >= current_left_distance) {
			int pos = random(0, avilable_models.size());
			auto model = avilable_models[pos];
			model->translateX(-ROAD_WIDHT_x2);
			model->translateZ(FAR);

			left_road.push_back(model);
			avilable_models.erase(avilable_models.begin() + pos);
			current_left_distance = random(MIN_DISTANCE, MAX_DISTANCE);
		}
	}

	if (!avilable_models.empty()) {
		if (right_road.empty() ||
			right_road.back()->distanceTo(glm::vec3(ROAD_WIDHT_x2, 0, FAR)) >= current_right_distance) {
			int pos = random(0, avilable_models.size());
			auto model = avilable_models[pos];
			model->translateX(ROAD_WIDHT_x2);
			model->translateZ(FAR);

			right_road.push_back(model);
			avilable_models.erase(avilable_models.begin() + pos);
			current_right_distance = random(MIN_DISTANCE, MAX_DISTANCE);
		}
	}

	if (left_road.front()->getPosition().z <= NEAR) {
		auto model = left_road.front();
		left_road.pop_front();
		avilable_models.push_back(model);
	}

	if (right_road.front()->getPosition().z <= NEAR) {
		auto model = right_road.front();
		right_road.pop_front();
		avilable_models.push_back(model);
	}
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Instalar el shader para dibujar los ejes, las normales y la luz
	ConstantIllumProgram::use();
	glEnable(GL_STENCIL_TEST);

	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	windshield->render(mats);

	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	render_models();

	glDisable(GL_STENCIL_TEST);

	// Desactivar la escritura al Z-buffer
	glDepthMask(GL_FALSE);
	// Ahora dibujamos los objetos translúcidos, en este orden: rojo, verde, azul
	// Ten en cuenta que el resultado no será el esperado, dependiendo del punto de vista
	glEnable(GL_BLEND);
	windshield->render(mats);
	glDisable(GL_BLEND);
	// Activar la escritura al Z-buffer
	glDepthMask(GL_TRUE);

	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	if (h == 0)
		h = 1;
	float ar = (float)w / h;

	mats->setMatrix(GLMatrices::PROJ_MATRIX,
		glm::perspective(glm::radians(10.0f), ar, .1f, FAR));
}

void MyRender::update_models(uint ms) {
	for (auto& elem : left_road) {
		elem->translate(0, 0, VELOCITY * ms);
	}

	for (auto& elem : right_road) {
		elem->translate(0, 0, VELOCITY * ms);
	}
}

void MyRender::update(uint ms) {
	update_models(ms);
}


int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.setInitWindowSize(800, 600);
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER | PGUPV::MULTISAMPLE | PGUPV::STENCIL_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
