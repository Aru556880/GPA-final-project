#include "src\Shader.h"
#include "src\SceneRenderer.h"
#include <GLFW\glfw3.h>
#include "src\MyImGuiPanel.h"

#include "src\MyCameraManager.h"
#include "src\Common.h"


#pragma comment (lib, "lib-vc2015\\glfw3.lib")
#pragma comment(lib, "assimp-vc141-mt.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "../externals/include/stb_image.h"

int FRAME_WIDTH = 1024;
int FRAME_HEIGHT = 512;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool initializeGL();
void resizeGL(GLFWwindow *window, int w, int h);
void paintGL();
void resize(const int w, const int h);

bool m_leftButtonPressed = false;
bool m_rightButtonPressed = false;
double cursorPos[2];



MyImGuiPanel* m_imguiPanel = nullptr;

void vsyncDisabled(GLFWwindow *window);

// ==============================================
SceneRenderer *defaultRenderer = nullptr;
ShaderProgram* defaultShaderProgram = new ShaderProgram();

IndoorSceneObject* m_indoorSO = nullptr;
INANOA::MyCameraManager* m_myCameraManager = nullptr;
// ==============================================

int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "rendering", nullptr, nullptr);
	if (window == nullptr){
		std::cout << "failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// load OpenGL function pointer
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetKeyCallback(window, keyCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetFramebufferSizeCallback(window, resizeGL);

	if (initializeGL() == false) {
		std::cout << "initialize GL failed\n";
		glfwTerminate();
		system("pause");
		return 0;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	// disable vsync
	//glfwSwapInterval(0);

	// start game-loop
	vsyncDisabled(window);
		

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void vsyncDisabled(GLFWwindow *window) {
	double previousTimeForFPS = glfwGetTime();
	int frameCount = 0;

	static int IMG_IDX = 0;

	while (!glfwWindowShouldClose(window)) {
		// measure speed
		const double currentTime = glfwGetTime();
		frameCount = frameCount + 1;
		const double deltaTime = currentTime - previousTimeForFPS;
		if (deltaTime >= 1.0) {
			m_imguiPanel->setAvgFPS(frameCount * 1.0);
			m_imguiPanel->setAvgFrameTime(deltaTime * 1000.0 / frameCount);

			// reset
			frameCount = 0;
			previousTimeForFPS = currentTime;
		}			

		glfwPollEvents();
		paintGL();
		glfwSwapBuffers(window);
	}
}



bool initializeGL(){
	// initialize shader program
	// vertex shader
	Shader* vsShader = new Shader(GL_VERTEX_SHADER);
	vsShader->createShaderFromFile("src\\shader\\oglVertexShader.glsl");
	std::cout << vsShader->shaderInfoLog() << "\n";

	// fragment shader
	Shader* fsShader = new Shader(GL_FRAGMENT_SHADER);
	fsShader->createShaderFromFile("src\\shader\\oglFragmentShader.glsl");
	std::cout << fsShader->shaderInfoLog() << "\n";

	// shader program
	ShaderProgram* shaderProgram = new ShaderProgram();
	shaderProgram->init();
	shaderProgram->attachShader(vsShader);
	shaderProgram->attachShader(fsShader);
	shaderProgram->checkStatus();
	if (shaderProgram->status() != ShaderProgramStatus::READY) {
		return false;
	}
	shaderProgram->linkProgram();

	vsShader->releaseShader();
	fsShader->releaseShader();
	
	delete vsShader;
	delete fsShader;
	// =================================================================
	// init renderer
	defaultRenderer = new SceneRenderer();
	if (!defaultRenderer->initialize(FRAME_WIDTH, FRAME_HEIGHT, shaderProgram)) {
		return false;
	}

	// =================================================================
	// initialize camera
	m_myCameraManager = new INANOA::MyCameraManager();
	m_myCameraManager->init(FRAME_WIDTH, FRAME_HEIGHT);
	
	// initialize terrain
	m_indoorSO = new IndoorSceneObject();
	m_indoorSO->init();
	defaultRenderer->appendIndoorSceneObject(m_indoorSO);
	// =================================================================	
	
	resize(FRAME_WIDTH, FRAME_HEIGHT);
	
	m_imguiPanel = new MyImGuiPanel(m_myCameraManager);
	
	return true;
}
void resizeGL(GLFWwindow *window, int w, int h){
	resize(w, h);
}

void paintGL(){
	// player camera
	m_myCameraManager->updatePlayerCamera();
	const glm::vec3 PLAYER_CAMERA_POSITION = m_myCameraManager->playerViewOrig();

	// prepare parameters
	const glm::mat4 playerVM = m_myCameraManager->playerViewMatrix();
	const glm::mat4 playerProjMat = m_myCameraManager->playerProjectionMatrix();
	const glm::vec3 playerViewOrg = m_myCameraManager->playerViewOrig();


	// (x, y, w, h)
	const glm::ivec4 playerViewport = m_myCameraManager->playerViewport();

	// =============================================
		
	// =============================================
	// start rendering
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	// start new frame
	defaultRenderer->setViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	defaultRenderer->startNewFrame();

	// rendering with player view		
	defaultRenderer->setViewport(playerViewport[0], playerViewport[1], playerViewport[2], playerViewport[3]);
	defaultRenderer->setView(playerVM);
	defaultRenderer->setProjection(playerProjMat);
	defaultRenderer->renderPass();

	// ===============================

	ImGui::Begin("My name is window");
	m_imguiPanel->update();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

////////////////////////////////////////////////
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		m_myCameraManager->mousePress(RenderWidgetMouseButton::M_LEFT, cursorPos[0], cursorPos[1]);
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		m_myCameraManager->mouseRelease(RenderWidgetMouseButton::M_LEFT, cursorPos[0], cursorPos[1]);
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		m_myCameraManager->mousePress(RenderWidgetMouseButton::M_RIGHT, cursorPos[0], cursorPos[1]);
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		m_myCameraManager->mouseRelease(RenderWidgetMouseButton::M_RIGHT, cursorPos[0], cursorPos[1]);
	}
}
void cursorPosCallback(GLFWwindow* window, double x, double y){
	cursorPos[0] = x;
	cursorPos[1] = y;

	m_myCameraManager->mouseMove(x, y);
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
	auto setKeyStatus = [](const RenderWidgetKeyCode code, const int action) {
		if (action == GLFW_PRESS) {
			m_myCameraManager->keyPress(code);
		}
		else if (action == GLFW_RELEASE) {
			m_myCameraManager->keyRelease(code);
		}
	};

	// =======================================
	if (key == GLFW_KEY_W) { setKeyStatus(RenderWidgetKeyCode::KEY_W, action); }
	else if (key == GLFW_KEY_A) { setKeyStatus(RenderWidgetKeyCode::KEY_A, action); }
	else if (key == GLFW_KEY_S) { setKeyStatus(RenderWidgetKeyCode::KEY_S, action); }
	else if (key == GLFW_KEY_D) { setKeyStatus(RenderWidgetKeyCode::KEY_D, action); }
	else if (key == GLFW_KEY_T) { setKeyStatus(RenderWidgetKeyCode::KEY_T, action); }
	else if (key == GLFW_KEY_Z) { setKeyStatus(RenderWidgetKeyCode::KEY_Z, action); }
	else if (key == GLFW_KEY_X) { setKeyStatus(RenderWidgetKeyCode::KEY_X, action); }
}
void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {}

void resize(const int w, const int h) {
	FRAME_WIDTH = w;
	FRAME_HEIGHT = h;

	m_myCameraManager->resize(w, h);
	defaultRenderer->resize(w, h);

}

texture_data loadImg(const char* path)
{
	texture_data texture;
	int n;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load(path, &texture.width, &texture.height, &n, 4);
	if (data != NULL)
	{
		texture.data = new unsigned char[texture.width * texture.height * 4 * sizeof(unsigned char)];
		memcpy(texture.data, data, texture.width * texture.height * 4 * sizeof(unsigned char));
		stbi_image_free(data);
	}
	else
	{
		cerr << "Cannot load image from: " << path << endl;
	}
	return texture;
}
