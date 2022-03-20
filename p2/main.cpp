
#include "PGUPV.h"
#include <deque>

using namespace PGUPV;

#define MIN_DISTANCE 20 //Mínima distancia a la que se puede colocar el próximo árbol
#define MAX_DISTANCE 40 //Máxima distancia a la que se puede colocar el próximo árbol
#define VELOCITY -60.f //Velocidad a la que se mueven los objetos
#define ROAD_WIDHT_x2 1.f //Grosor de la carretera
#define FAR 200.f //Distancia máxima
#define NEAR 0.1f //Distancia mínima
#define COLOR glm::vec4(0, 0, 1, 0.3) //Color del parabrisas

std::map<std::string, glm::vec4> loadMTL(std::string path,
	std::string file_name, glm::vec4 new_color) {
	std::stringstream ss;
	std::ifstream in_file(path + file_name);
	std::string line = "";
	std::string prefix = "";

	std::string current_key;
	std::map<std::string, glm::vec4> dict_material;

	//Verificar si falla la apertura del archivo
	if (!in_file.is_open()) {
		throw "ERROR::MTLLOADER::Could not open file.";
	}

	//Si se le define un nuevo color new_color, establecerlo como color
	//para todos los materiales
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

	//En otro caso definir para cada material su color Kd correspondiente
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

	//Verificar si falla la apertura del archivo
	if (!in_file.is_open()) {
		throw "ERROR::OBJLOADER::Could not open file.";
	}

	//Leer cada linea y parsear el documento
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
			//Al encontrarnos una cara, se indexa en las listas vertex_positions,
			//vertex_texcoords, vertex_normals y vertex_materials según corresponda;
			//dejando en positions, texcoords, normals y colors los elementos de cada
			//cara del modelo

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

	//Establecer los elementos necesarios para definir la malla
	auto mesh = std::make_shared<Mesh>();
	mesh->addVertices(positions);
	if (!normals.empty()) mesh->addNormals(normals);
	if (!texcoords.empty()) mesh->addTexCoord(0, texcoords);
	mesh->addColors(colors);
	mesh->addDrawCommand(new PGUPV::DrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(positions.size())));

	auto model = std::make_shared<Model>();
	model->addMesh(mesh);

	in_file.close();
	std::cout << "OBJ file loaded!" << "\n";
	return model;
}

int random(int a, int b) {
	//Random [a, b)
	return a + (std::rand() % (b - a));
}


class GameObject {
public:
	GameObject() {};
	GameObject(std::shared_ptr<Model> model) : _model(model) {};

	void render(std::shared_ptr<GLMatrices> mats);

	//Definir modelos
	void setModel(std::shared_ptr<Model> model) {
		_model = model;
	};

	//Añadir valores a los ángulos de rotación
	void rotate(float x, float y, float z) {
		angle_x += x; angle_y += y; angle_z += z;
	};

	//Definir ángulo de rotación
	void rotateX(float x) {
		angle_x = x;
	};
	void rotateY(float y) {
		angle_y = y;
	};
	void rotateZ(float z) {
		angle_z = z;
	};

	//Añadir valores a la posición del objeto
	void translate(float x, float y, float z) {
		current_x += x; current_y += y; current_z += z;
	};

	//Definir posición de cada componente
	void translateX(float x) {
		current_x = x;
	};
	void translateY(float y) {
		current_y = y;
	};
	void translateZ(float z) {
		current_z = z;
	};

	//Escalar el objeto a determinado tamaño
	void scaleTo(float size) {
		this->size = size;
	};

	//Distancia de la posición actual a un punto
	float distanceTo(glm::vec3 point);

	//Devolver la posición
	glm::vec3 getPosition() {
		return glm::vec3(current_x, current_y, current_z);
	};

	//Devolver ángulos de rotación
	glm::vec3 getRotation() {
		return glm::vec3(angle_x, angle_y, angle_z);
	};

	//Definir Tag
	void setTag(std::string tag) {
		this->tag = tag;
	}

	//Devolver Tag
	std::string getTag() {
		return tag;
	}

private:
	std::shared_ptr<Model> _model;

	//Posición
	float current_x = 0;
	float current_y = 0;
	float current_z = 0;

	//Ángulos de rotación
	float angle_x = 0;
	float angle_y = 0;
	float angle_z = 0;

	//Tamaño
	float size = 0;

	//Tag
	std::string tag = "";
};

void GameObject::render(std::shared_ptr<GLMatrices> mats) {
	//Funcón para hacer render a un modelo

	mats->pushMatrix(GLMatrices::MODEL_MATRIX);

	//Traslación
	mats->translate(GLMatrices::MODEL_MATRIX,
		current_x, current_y, current_z);

	//Rotación en cada eje
	mats->rotate(GLMatrices::MODEL_MATRIX, angle_x, glm::vec3{ 1.0f, 0.0f, 0.0f });
	mats->rotate(GLMatrices::MODEL_MATRIX, angle_y, glm::vec3{ 0.0f, 1.0f, 0.0f });
	mats->rotate(GLMatrices::MODEL_MATRIX, angle_z, glm::vec3{ 0.0f, 0.0f, 1.0f });

	//Escalado
	if (size > 0) {
		mats->scale(GLMatrices::MODEL_MATRIX, glm::vec3(size / _model->maxDimension()));
	}

	//Renderizar
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

	//Distancias actuales a la que se colocarán los próximos árboles
	float current_left_distance = 0;
	float current_right_distance = 0;

	std::shared_ptr<GameObject> windshield; //parabrisas

	std::vector<std::shared_ptr<GameObject>> avilable_models; //árboles disponibles
	std::deque<std::shared_ptr<GameObject>> left_road; //árboles del carril izquierdo
	std::deque<std::shared_ptr<GameObject>> right_road; //árboles del carril derecho

	void setup_models();
	void render_models();
	void update_models(uint ms);
};

void MyRender::setup_models() {
	//Cargar objetos
	auto model1 = loadOBJ("../recursos/modelos/trees/tree1/", "tree1.obj");
	auto model2 = loadOBJ("../recursos/modelos/trees/tree2/", "tree2.obj");
	auto model3 = loadOBJ("../recursos/modelos/trees/tree3/", "tree3.obj");

	//Cargar el parabrisas pero definiendo su color 
	//como COLOR (color azulado traslúcido)
	auto windshield_model = loadOBJ("../recursos/modelos/windshield/", 
		"windshield.obj", COLOR);
	
	//Crear parabrisas a partir del modelo importado
	windshield = std::make_shared<GameObject>(windshield_model);
	windshield->scaleTo(0.8);

	//Crear árboles de diferentes tamaños
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

	//Definir matriz de vista
	mats->setMatrix(GLMatrices::VIEW_MATRIX,
		glm::lookAt(glm::vec3(0.f, 0.2f, -5.f), glm::vec3(0.f, 0.1f, 0.f),
			glm::vec3(0.0f, 1.0f, 0.0f)));

	//Establecer parámetros de blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MyRender::render_models() {
	//Renderizar modelos del camino izquierdo
	for (auto& elem : left_road) {
		elem->render(mats);
	}

	//Renderizar modelos del camino derecho
	for (auto& elem : right_road) {
		elem->render(mats);
	}

	//Si existen modelos disponibles y se ha superado la distancia
	//desde el punto de partida hasta el último modelo que ha salido,
	//agregar un nuevo modelo a la cola izquierda
	if (!avilable_models.empty()) {
		if (left_road.empty() || 
			left_road.back()->distanceTo(glm::vec3(-ROAD_WIDHT_x2, 0, FAR)) >= current_left_distance) {
			//Seleccionar un modelo disponible de forma aleatoria
			int pos = random(0, avilable_models.size());
			auto model = avilable_models[pos];
			model->translateX(-ROAD_WIDHT_x2);
			model->translateZ(FAR);

			//Quitarlo de los disponibles y colocarlo en la cola izquierda
			left_road.push_back(model);
			avilable_models.erase(avilable_models.begin() + pos);

			//Determinar una nueva distancia para el próximo modelo
			current_left_distance = random(MIN_DISTANCE, MAX_DISTANCE);
		}
	}

	//Idem a lo anterior, pero para la cola derecha
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

	//Si el primer modelo ha superado el punto de llegada
	//quitarlo de la cola izquierda y colocarlo en los disponibles
	if (left_road.front()->getPosition().z <= NEAR) {
		auto model = left_road.front();
		left_road.pop_front();
		avilable_models.push_back(model);
	}

	//Idem a lo anterior pero para la cola derecha
	if (right_road.front()->getPosition().z <= NEAR) {
		auto model = right_road.front();
		right_road.pop_front();
		avilable_models.push_back(model);
	}
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//Instalar el shader para dibujar objetos con color
	ConstantIllumProgram::use();

	//Activar el stencil
	glEnable(GL_STENCIL_TEST);

	//Desactivar escritura en el z-buffer y color-buffer
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	//Hacer uno los píxeles del stencil
	//que corresponden a la forma del parabrisas
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	windshield->render(mats);

	//Activar escritura en el z-buffer y color-buffer
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	//Hacer que los modelos solo se rendericen
	//en los píxeles del parabrisas
	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	render_models();

	//Desactivar el stencil
	glDisable(GL_STENCIL_TEST);

	//Desactivar la escritura al z-buffer
	glDepthMask(GL_FALSE);

	//Activar blend para renderizar nuevamente el parabrisas, 
	//pero esta vez mostrando su  color traslúcido
	glEnable(GL_BLEND);
	windshield->render(mats);
	glDisable(GL_BLEND);

	//Activar la escritura al z-buffer
	glDepthMask(GL_TRUE);

	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	if (h == 0) h = 1;
	float ar = (float)w / h;

	//Actualizar la matriz de proyección según
	//el nuevo aspect ratio de la ventana
	mats->setMatrix(GLMatrices::PROJ_MATRIX,
		glm::perspective(glm::radians(10.0f), ar, NEAR, FAR));
}

void MyRender::update_models(uint ms) {
	//Incrementar la distancia de los elementos según
	//la velocidad definida en función del tiempo transcurrido

	for (auto& elem : left_road) {
		elem->translate(0, 0, VELOCITY * ms / 1000.0f);
	}

	for (auto& elem : right_road) {
		elem->translate(0, 0, VELOCITY * ms / 1000.0f);
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
