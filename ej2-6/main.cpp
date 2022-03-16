#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"
#include "GUI3.h"

using namespace PGUPV;

using glm::vec3;
using std::vector;

/*

Aplicación de demostración del uso de los distintos parámetros del stencil.
Podrás cambiar la configuración del stencil y ver los resultados en tiempo real.

*/

class MyRender : public Renderer {
public:
	MyRender() : colorInterp(std::vector<KeyFrame<float, glm::vec4>> {
		{0.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ 0.5f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ 1.0f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) }},
		ONE_TIME) { };
		void setup(void) override;
		void render(void) override;
		void reshape(uint w, uint h) override;
		void update(uint) override;
private:
	std::shared_ptr<GLMatrices> mats;
	Axes axes;
	Rect rect;
	MultiLinearInterpolator<float, glm::vec4> colorInterp;

	void buildGUI();
	std::shared_ptr<CheckBoxWidget> enableStencil;
	std::shared_ptr<ListBoxWidget<GLenum>> stencilFunc;
	std::shared_ptr<IntInputWidget> stencilRef;
	std::shared_ptr<IntInputWidget> stencilMask;
	std::shared_ptr<ListBoxWidget<GLenum>> stencilOpFail, stencilOpZFail, stencilOpPass;
	std::shared_ptr<IntInputWidget> numPoly;
};

void MyRender::buildGUI() {
	auto panel = addPanel("Stencil options");
	panel->setPosition(500, 10);
	panel->setSize(280, 450);

	enableStencil = std::make_shared<CheckBoxWidget>("Stencil test", true);
	glEnable(GL_STENCIL_TEST);
	panel->addWidget(enableStencil);

	panel->addWidget(std::make_shared<Separator>());
	panel->addWidget(std::make_shared<Label>("Stencil Func"));

	// Selector de la funci�n de test (GL_NEVER, GL_LESS, etc)
	// Creamos un vector de cadenas con las opciones que queremos mostrar en una lista
	std::vector<std::string> stencilFuncNames;
	std::vector<GLenum> stencilFuncEnums;

#define SF(F) stencilFuncNames.push_back(#F); stencilFuncEnums.push_back(F);
	SF(GL_NEVER)
		SF(GL_LESS)
		SF(GL_EQUAL)
		SF(GL_LEQUAL)
		SF(GL_GREATER)
		SF(GL_NOTEQUAL)
		SF(GL_GEQUAL)
		SF(GL_ALWAYS)
#undef SF

	auto setStencilFunc = [this]() {
		glStencilFunc(stencilFunc->getSelectedUserData(), stencilRef->get(), stencilMask->get());
	};


	// Creamos un listbox con los elementos anteriores
	stencilFunc = std::make_shared<ListBoxWidget<GLenum>>("Stencil func", stencilFuncNames, 7 /* Default value: GL_ALWAYS */);
	stencilFunc->setNumVisibleElements(4);
	stencilFunc->setUserData(stencilFuncEnums);
	stencilFunc->getValue().addListener([setStencilFunc](const int &) {
		setStencilFunc();
	});
	panel->addWidget(stencilFunc);

	stencilRef = std::make_shared<IntInputWidget>("Reference", 0, 0, 255);
	stencilRef->getValue().addListener([setStencilFunc](const int &) {
		setStencilFunc();
	});
	panel->addWidget(stencilRef);

	stencilMask = std::make_shared<IntInputWidget>("Mask", 255, 0, 255);
	stencilMask->getValue().addListener([setStencilFunc](const int &) {
		setStencilFunc();
	});
	panel->addWidget(stencilMask);


	panel->addWidget(std::make_shared<Separator>());
	panel->addWidget(std::make_shared<Label>("Stencil Op"));

	// Selector de la operaci�n de actualizaci�n (GL_KEEP, GL_ZERO, etc)
	// Creamos un vector de cadenas con las opciones que queremos mostrar en una lista
	std::vector<std::string> stencilOpNames;
	std::vector<GLenum> stencilOpEnums;

#define SO(F) stencilOpNames.push_back(#F); stencilOpEnums.push_back(F);
	SO(GL_KEEP)
		SO(GL_ZERO)
		SO(GL_REPLACE)
		SO(GL_INCR)
		SO(GL_INCR_WRAP)
		SO(GL_DECR)
		SO(GL_DECR_WRAP)
		SO(GL_INVERT)
#undef SO


		auto setStencilOp = [this]() {
		glStencilOp(stencilOpFail->getSelectedUserData(),
			stencilOpZFail->getSelectedUserData(),
			stencilOpPass->getSelectedUserData());
	};

	// Creamos un listbox con los elementos anteriores
	stencilOpFail = std::make_shared<ListBoxWidget<GLenum>>("Fail Op", stencilOpNames, 0);
	stencilOpFail->setNumVisibleElements(3);
	stencilOpFail->setUserData(stencilOpEnums);
	stencilOpFail->getValue().addListener([setStencilOp](const int &) {
		setStencilOp();
	});
	panel->addWidget(stencilOpFail);

	stencilOpZFail = std::make_shared<ListBoxWidget<GLenum>>("Z-Fail Op", stencilOpNames, 0);
	stencilOpZFail->setNumVisibleElements(3);
	stencilOpZFail->setUserData(stencilOpEnums);
	stencilOpZFail->getValue().addListener([setStencilOp](const int &) {
		setStencilOp();
	});
	panel->addWidget(stencilOpZFail);

	stencilOpPass = std::make_shared<ListBoxWidget<GLenum>>("Pass Op", stencilOpNames, 0);
	stencilOpPass->setNumVisibleElements(3);
	stencilOpPass->setUserData(stencilOpEnums);
	stencilOpPass->getValue().addListener([setStencilOp](const int &) {
		setStencilOp();
	});
	panel->addWidget(stencilOpPass);

	// Clear stencil value
	panel->addWidget(std::make_shared<Separator>());
	auto clearStencilValue = std::make_shared<IntInputWidget>("Clear value", 0, 0, 255);
	clearStencilValue->getValue().addListener([](const int &val) {
		glClearStencil(val);
	});
	panel->addWidget(clearStencilValue);

	auto panel2 = addPanel("Otros");
	panel2->setPosition(15, 15);
	panel2->setSize(260, 100);

	numPoly = std::make_shared<IntInputWidget>("Num. quads", 4, 1, 64);
	panel2->addWidget(numPoly);

	auto writeStencilMask = std::make_shared<IntInputWidget>("Write stencil mask", 255, 0, 255);
	panel2->addWidget(writeStencilMask);
	writeStencilMask->getValue().addListener([](const int &mask) {
		glStencilMask(mask);
	});

	auto depthTest = std::make_shared<CheckBoxWidget>("Depth test", true);
	panel2->addWidget(depthTest);
	depthTest->getValue().addListener([](const bool &zbuffer) {
		if (zbuffer) {
			glEnable(GL_DEPTH_TEST);
		}
		else {
			glDisable(GL_DEPTH_TEST);
		}
	});
}

void MyRender::setup() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Habilitamos el z-buffer
	glEnable(GL_DEPTH_TEST);

	mats = PGUPV::GLMatrices::build();

	// Establecemos una c�mara que nos permite explorar el objeto desde cualquier
	// punto
	setCameraHandler(std::make_shared<OrbitCameraHandler>());

	buildGUI();
	App::getInstance().getWindow().showGUI(true);
}

void MyRender::render() {
	// Como puede ser que haya una m�scara de escritura en el stencil definida por el usuario,
	// pero queremos borrar su contenido al principio siempre:
	StencilMaskState maskState;
	glStencilMask(255);
	// Borramos los buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	maskState.restore();

	// Le pedimos a la c�mara que nos de la matriz de la vista, que codifica la
	// posici�n y orientaci�n actuales de la c�mara.
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());


	// Para dibujar los ejes deshabilitamos el stencil (por si est� habilitado)
	StencilTestEnabledState stencilOn;
	glDisable(GL_STENCIL_TEST);
	// Dibujamos los ejes
	ConstantIllumProgram::use();
	axes.render();
	stencilOn.restore();

	ConstantUniformColorProgram::use();

	if (enableStencil->get())
		glEnable(GL_STENCIL_TEST);

	// Dibujamos los objetos
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	float offset = numPoly->get() == 1 ? 0.0f : 1.0f / (numPoly->get() - 1);
	for (int i = 0; i < numPoly->get(); i++) {
		auto c = colorInterp.interpolate(i * offset);
		ConstantUniformColorProgram::setColor(c);
		rect.render();
		mats->translate(GLMatrices::MODEL_MATRIX, 0.0f, 0.0f, offset);
	}
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	if (enableStencil->get())
		glDisable(GL_STENCIL_TEST);

	// Si la siguiente llamada falla, quiere decir que OpenGL se encuentra en
	// estado err�neo porque alguna de las operaciones que ha ejecutado
	// recientemente (despu�s de la �ltima llamada a CHECK_GL) ha tenido alg�n
	// problema. Revisa tu c�digo.
	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
}

// Este m�todo se ejecuta una vez por frame, antes de llamada a render. Recibe el 
// n�mero de milisegundos que han pasado desde la �ltima vez que se llam�, y se suele
// usar para hacer animaciones o comprobar el estado de los dispositivos de entrada
void MyRender::update(uint ) {
	// Si el usuario ha pulsado el espacio, ponemos la c�mara en su posici�n inicial
	if (App::isKeyUp(PGUPV::KeyCode::Space)) {
		getCameraHandler()->resetView();
	}
}

int main(int argc, char *argv[]) {
	App &myApp = App::getInstance();
	myApp.setInitWindowSize(800, 600);
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
		PGUPV::STENCIL_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
