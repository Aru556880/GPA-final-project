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

GLFWwindow* window;

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

	window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "rendering", nullptr, nullptr);
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
	// =================================================================
	// init renderer
	defaultRenderer = new SceneRenderer();
	if (!defaultRenderer->initialize(FRAME_WIDTH, FRAME_HEIGHT)) {
		return false;
	}

	// =================================================================
	// initialize camera
	m_myCameraManager = new INANOA::MyCameraManager();
	m_myCameraManager->init(FRAME_WIDTH, FRAME_HEIGHT);
	
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
	else if (key == GLFW_KEY_RIGHT) { setKeyStatus(RenderWidgetKeyCode::KEY_RIGHT, action); }
	else if (key == GLFW_KEY_LEFT) { setKeyStatus(RenderWidgetKeyCode::KEY_LEFT, action); }
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

void LoadMeshModel(vector<MyMesh>& shapes, string filePath, uint start_mesh, uint end_mesh) {
	const aiScene* scene;

	scene = aiImportFile(filePath.c_str(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

	if (!scene)
	{
		std::cerr << "Error loading file: " << filePath << std::endl;
		std::cerr << "Assimp error: " << aiGetErrorString() << std::endl;
		return;
	}
	else
	{
		cout << "File loaded successfully!" << std::endl;
	}

	cout << "Mesh: " << scene->mNumMeshes << endl;
	cout << "Material: " << scene->mNumMaterials << endl;

	// load material
	vector<Material> materials;
	bool use_normalMap = false;
	for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
		Material Material;
		aiMaterial* material = scene->mMaterials[i];
		aiString texturePath;
		aiColor3D color;
		float shininess;

		// diffuse texture
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
		{
			string imgPath = "assets/indoor/" + string(texturePath.C_Str());
			texture_data tdata = loadImg(imgPath.c_str());
			glGenTextures(1, &Material.diffuse_tex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Material.diffuse_tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);

			glGenerateMipmap(GL_TEXTURE_2D);

		}
		else
		{
			glGenTextures(1, &Material.diffuse_tex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Material.diffuse_tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			material->Get(AI_MATKEY_COLOR_DIFFUSE, color);

			unsigned char colorData[] = {
				color.r * 255, color.g * 255, color.b * 255, 255,
				color.r * 255, color.g * 255, color.b * 255, 255,
				color.r * 255, color.g * 255, color.b * 255, 255,
				color.r * 255, color.g * 255, color.b * 255, 255,
			};
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorData );
			glGenerateMipmap(GL_TEXTURE_2D);
			
		}

		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		Material.Ka = vec3(color.r, color.g, color.b);

		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		Material.Kd = vec3(color.r, color.g, color.b);

		material->Get(AI_MATKEY_COLOR_SPECULAR, color);
		Material.Ks = vec3(color.r, color.g, color.b);

		material->Get(AI_MATKEY_SHININESS, shininess);
		Material.shininess = shininess;

		// normal map setup
		aiString normalTexturePath;
		if (material->GetTexture(aiTextureType_HEIGHT, 0, &normalTexturePath) == aiReturn_SUCCESS)
		{
			string imgPath = "assets/indoor/" + string(normalTexturePath.C_Str());
			Material.useNormalMap = true;

			texture_data tdata = loadImg(imgPath.c_str());
			glGenTextures(1, &Material.normalmap_tex);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, Material.normalmap_tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			Material.useNormalMap = false;
		}

		materials.push_back(Material);
	}
	std::cout << "finish material loading!" << endl;

	// load geometry and save them with material
	int meshnum = glm::min(scene->mNumMeshes, end_mesh);
	for (unsigned int i = start_mesh; i < meshnum; ++i) {
		aiMesh* mesh = scene->mMeshes[i];

		MyMesh shape;
		glGenVertexArrays(1, &shape.vao);

		vector<float> vertices, texcoords, normals, mytangent;
		vector<unsigned int> indices;
		//cout << i << " vertex: " << mesh->mNumVertices << endl;
		for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
		{
			//if (i == 0)
				//cout << mesh->mVertices[v][0] << ", " << mesh->mVertices[v][1] << ", " << mesh->mVertices[v][2] << endl;
			vertices.push_back(mesh->mVertices[v][0]);
			vertices.push_back(mesh->mVertices[v][1]);
			vertices.push_back(mesh->mVertices[v][2]);

			normals.push_back(mesh->mNormals[v][0]);
			normals.push_back(mesh->mNormals[v][1]);
			normals.push_back(mesh->mNormals[v][2]);

			if (mesh->mTextureCoords[0]) {
				// Check if texture coordinates exist
				texcoords.push_back(mesh->mTextureCoords[0][v][0]);
				texcoords.push_back(mesh->mTextureCoords[0][v][1]);

			}
			else {
				texcoords.push_back(0.0f);
				texcoords.push_back(0.0f);
			}

			mytangent.push_back(mesh->mTangents[i].x);
			mytangent.push_back(mesh->mTangents[i].y);
			mytangent.push_back(mesh->mTangents[i].z);
		}

		// create 1 ibo to hold data
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
		{
			indices.push_back(mesh->mFaces[f].mIndices[0]);
			indices.push_back(mesh->mFaces[f].mIndices[1]);
			indices.push_back(mesh->mFaces[f].mIndices[2]);
		}

		// Compute tangent
		//float* tangents = new float[normals.size()];
		vector<float> tangents(normals.size(), 0.0f);
		for (unsigned int i = 0; i < indices.size(); i += 3) {
			int idx0 = indices[i];
			int idx1 = indices[i + 1];
			int idx2 = indices[i + 2];

			vec3 p1(vertices[idx0 * 3 + 0], vertices[idx0 * 3 + 1], vertices[idx0 * 3 + 2]);
			vec3 p2(vertices[idx1 * 3 + 0], vertices[idx1 * 3 + 1], vertices[idx1 * 3 + 2]);
			vec3 p3(vertices[idx2 * 3 + 0], vertices[idx2 * 3 + 1], vertices[idx2 * 3 + 2]);

			vec3 n1(normals[idx0 * 3 + 0], normals[idx0 * 3 + 1], normals[idx0 * 3 + 2]);
			vec3 n2(normals[idx1 * 3 + 0], normals[idx1 * 3 + 1], normals[idx1 * 3 + 2]);
			vec3 n3(normals[idx2 * 3 + 0], normals[idx2 * 3 + 1], normals[idx2 * 3 + 2]);

			vec2 uv1(texcoords[idx0 * 2 + 0], texcoords[idx0 * 2 + 1]);
			vec2 uv2(texcoords[idx1 * 2 + 0], texcoords[idx1 * 2 + 1]);
			vec2 uv3(texcoords[idx2 * 2 + 0], texcoords[idx2 * 2 + 1]);

			vec3 dp1 = p2 - p1;
			vec3 dp2 = p3 - p1;

			vec2 duv1 = uv2 - uv1;
			vec2 duv2 = uv3 - uv1;

			float r = 1.0f / (duv1[0] * duv2[1] - duv1[1] * duv2[0]);

			vec3 t;
			t[0] = (dp1[0] * duv2[1] - dp2[0] * duv1[1]) * r;
			t[1] = (dp1[1] * duv2[1] - dp2[1] * duv1[1]) * r;
			t[2] = (dp1[2] * duv2[1] - dp2[2] * duv1[1]) * r;

			vec3 t1 = glm::cross(n1, t);
			vec3 t2 = glm::cross(n2, t);
			vec3 t3 = glm::cross(n3, t);

			tangents[idx0 * 3 + 0] += t1[0];
			tangents[idx0 * 3 + 1] += t1[1];
			tangents[idx0 * 3 + 2] += t1[2];

			tangents[idx1 * 3 + 0] += t2[0];
			tangents[idx1 * 3 + 1] += t2[1];
			tangents[idx1 * 3 + 2] += t2[2];

			tangents[idx2 * 3 + 0] += t3[0];
			tangents[idx2 * 3 + 1] += t3[1];
			tangents[idx2 * 3 + 2] += t3[2];
		}

		glBindVertexArray(shape.vao);

		glGenBuffers(1, &shape.vbo_position);
		glGenBuffers(1, &shape.vbo_normal);
		glGenBuffers(1, &shape.vbo_texcoord);
		glGenBuffers(1, &shape.vbo_tangent);
		glGenBuffers(1, &shape.ibo);

		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_position);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_normal);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GL_FLOAT), normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_texcoord);
		glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GL_FLOAT), texcoords.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		// mytangents: obtained by aiProcess_CalcTangentSpace
		// tangents: calculated manually as in lecture program
		// I want to apply tangent to other indoor models but te rendered result will be weird...
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_tangent);
		glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(GL_FLOAT), tangents.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GL_UNSIGNED_INT), indices.data(), GL_STATIC_DRAW);

		int materialID = mesh->mMaterialIndex;
		shape.drawCount = mesh->mNumFaces * 3;
		shape.material = materials[materialID];

		/* DEBUG: check normal map is used or not
		aiMaterial* material = scene->mMaterials[materialID];
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);
		std::cout << "Material " << i << " name: " << materialName.C_Str() << std::endl;
		cout << "Normap: " << shape.material.useNormalMap << endl;
		*/

		shapes.push_back(shape);
	}

	aiReleaseImport(scene);
	glBindVertexArray(0);
}
