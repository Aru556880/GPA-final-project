#include "src\Shader.h"
#include "src\SceneRenderer.h"
#include <GLFW\glfw3.h>
#include "src\MyImGuiPanel.h"

#include "src\ViewFrustumSceneObject.h"
#include "src\terrain\MyTerrain.h"
#include "src\MyCameraManager.h"
#include "src\MyPoissonSample.h"

#pragma comment (lib, "lib-vc2015\\glfw3.lib")
#pragma comment(lib, "assimp-vc141-mt.lib")

#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
#define STB_IMAGE_IMPLEMENTATION
#include "../externals/include/stb_image.h"
using namespace std;
using namespace glm;

int FRAME_WIDTH = 1024;
int FRAME_HEIGHT = 512;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool initializeGL();
void resizeGL(GLFWwindow* window, int w, int h);
void paintGL();
void resize(const int w, const int h);

bool m_leftButtonPressed = false;
bool m_rightButtonPressed = false;
double cursorPos[2];

MyImGuiPanel* m_imguiPanel = nullptr;

void vsyncDisabled(GLFWwindow* window);
void initSingleObjects();

// ==============================================
SceneRenderer* defaultRenderer = nullptr;
ShaderProgram* defaultShaderProgram = new ShaderProgram();

ViewFrustumSceneObject* m_viewFrustumSO = nullptr;
MyTerrain* m_terrain = nullptr;
INANOA::MyCameraManager* m_myCameraManager = nullptr;
// ==============================================

// image textures properties
const int NUM_INSTANCE_TEXTURE = 5;
const int IMG_WIDTH = 1024;
const int IMG_HEIGHT = 1024;
const int IMG_CHANNEL = 4; // Assuming RGBA
unsigned int textureArrayHandle;	// the handle for the instance objects texture array

// import instance properties
MyPoissonSample* sample0 = MyPoissonSample::fromFile("assets/poissonPoints_621043_after.ppd2");
MyPoissonSample* sample1 = MyPoissonSample::fromFile("assets/poissonPoints_1010.ppd2");
MyPoissonSample* sample2 = MyPoissonSample::fromFile("assets/poissonPoints_2797.ppd2");
MyPoissonSample* sample3 = MyPoissonSample::fromFile("assets/cityLots_sub_1.ppd2");
MyPoissonSample* sample4 = MyPoissonSample::fromFile("assets/cityLots_sub_0.ppd2");

// number of samples in the spatial samples for mesh positions
const int NUM_SAMPLE0 = sample0->m_numSample;
const int NUM_SAMPLE1 = sample1->m_numSample;
const int NUM_SAMPLE2 = sample2->m_numSample;
const int NUM_SAMPLE3 = sample3->m_numSample;
const int NUM_SAMPLE4 = sample4->m_numSample;
int TOTAL_INSTANCE = NUM_SAMPLE0 + NUM_SAMPLE1 + NUM_SAMPLE2 + NUM_SAMPLE3 + NUM_SAMPLE4;

///////////////////////////////////
// mesh struct for single objects (plane, stone)
struct Mesh {
	vector<float> vertices;         // vertex data
	vector<unsigned int> indices;   // index data
	unsigned int textureID;         // texture ID for this mesh
	unsigned int VAO, VBO, EBO;
	size_t count;                   // # of indices
};
// texture data
struct texture_data {
	unsigned char* data;  // Pointer to image data
	int width;            // Texture width
	int height;           // Texture height
};

texture_data load_Image(const char* path) {
	texture_data texture;
	int n;
	//stbi_set_flip_vertically_on_load(true);  // flip vertically for OpenGL
	unsigned char* data = stbi_load(path, &texture.width, &texture.height, &n, 4); // 4 for RGBA

	if (data != NULL) {
		texture.data = new unsigned char[texture.width * texture.height * 4];
		memcpy(texture.data, data, texture.width * texture.height * 4 * sizeof(unsigned char));
		stbi_image_free(data);
		cout << "Loaded texture:" << path << endl;
	}
	else {
		cerr << "Failed to load texture: " << path << endl;
	}

	return texture;
}

// generate gl textures
unsigned int generateOpenGLTexture(const char* path, int id) {
	texture_data texture = load_Image(path);

	unsigned int textureID = id;

	cout << textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (texture.data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.data);
	}
	return textureID;
}

// for loading in plane/stone models
Mesh loadSingleModel(const std::string& path, int tex_type) {
	Mesh resultMesh;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "Error: " << importer.GetErrorString() << std::endl;
		return resultMesh; // Return an empty Mesh object
	}

	// Process the first node with a mesh
	std::function<void(aiNode*, const aiScene*, bool&)> processNode = [&](aiNode* node, const aiScene* scene, bool& meshProcessed) {
		if (meshProcessed) return; // Stop if we've already processed a mesh

		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			// Temporary storage for normals, tangents, and bitangents
			std::vector<vec3> normals(mesh->mNumVertices, vec3(0.0f, 0.0f, 0.0f));
			std::vector<vec3> tangents(mesh->mNumVertices, vec3(0.0f, 0.0f, 0.0f));
			std::vector<vec3> bitangents(mesh->mNumVertices, vec3(0.0f, 0.0f, 0.0f));

			// Calculate normals, tangents, and bitangents
			for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
				aiFace face = mesh->mFaces[f];
				if (face.mNumIndices == 3) { // Ensure the face is a triangle
					aiVector3D v0 = mesh->mVertices[face.mIndices[0]];
					aiVector3D v1 = mesh->mVertices[face.mIndices[1]];
					aiVector3D v2 = mesh->mVertices[face.mIndices[2]];

					// Calculate edge vectors
					vec3 edge1 = vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
					vec3 edge2 = vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

					// Calculate face normal
					vec3 faceNormal = normalize(cross(edge1, edge2));

					// Update vertex normals
					normals[face.mIndices[0]] += faceNormal;
					normals[face.mIndices[1]] += faceNormal;
					normals[face.mIndices[2]] += faceNormal;

					// Extract 2D texture coordinates from aiVector3D
					aiVector3D uv0 = mesh->mTextureCoords[0][face.mIndices[0]];
					aiVector3D uv1 = mesh->mTextureCoords[0][face.mIndices[1]];
					aiVector3D uv2 = mesh->mTextureCoords[0][face.mIndices[2]];

					vec2 deltaUV1 = vec2(uv1.x - uv0.x, uv1.y - uv0.y);
					vec2 deltaUV2 = vec2(uv2.x - uv0.x, uv2.y - uv0.y);

					// Calculate tangent and bitangent
					vec3 tangent, bitangent;
					float denom = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
					if (fabs(denom) < 1e-6) { // Avoid division by zero
						tangent = vec3(1.0f, 0.0f, 0.0f); // Arbitrary fallback
						bitangent = vec3(0.0f, 1.0f, 0.0f);
						continue; // Skip to the next triangle
					}
					else {
						float f = 1.0f / denom;
						tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
						bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
					}
					//cout << bitangent[0] << endl;

					// Accumulate tangents and bitangents
					tangents[face.mIndices[0]] += tangent;
					tangents[face.mIndices[1]] += tangent;
					tangents[face.mIndices[2]] += tangent;

					bitangents[face.mIndices[0]] += bitangent;
					bitangents[face.mIndices[1]] += bitangent;
					bitangents[face.mIndices[2]] += bitangent;
				}
			}

			// Normalize the normals, tangents, and bitangents
			for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
				normals[v] = normalize(normals[v]);
				tangents[v] = normalize(tangents[v]);
				bitangents[v] = normalize(bitangents[v]);

				// Ensure the tangent and bitangent are orthogonal (optional but recommended)
				bitangents[v] = normalize(cross(normals[v], tangents[v]));

				//if (bitangents[v][0] > 0.0) cout << bitangents[v][0] << endl;
			}

			// handeling textures
			string texturePath;
			if (tex_type == 0) {		// airplane
				texturePath = "assets/Airplane_smooth_DefaultMaterial_BaseMap.jpg";
			}
			else if (tex_type == 1) {	// stone
				texturePath = "assets/MagicRock/StylMagicRocks_AlbedoTransparency.png";
			}
			resultMesh.textureID = generateOpenGLTexture(texturePath.c_str(), resultMesh.textureID);
			cout << "TextureID: " << resultMesh.textureID << std::endl;

			// Push vertices data (position, normal, tangent, bitangent, texture coords)
			for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
				// Positions
				resultMesh.vertices.push_back(mesh->mVertices[v].x);
				resultMesh.vertices.push_back(mesh->mVertices[v].y);
				resultMesh.vertices.push_back(mesh->mVertices[v].z);

				// Normals
				resultMesh.vertices.push_back(normals[v].x);
				resultMesh.vertices.push_back(normals[v].y);
				resultMesh.vertices.push_back(normals[v].z);

				// Texture coordinates
				if (mesh->mTextureCoords[0]) {
					resultMesh.vertices.push_back(mesh->mTextureCoords[0][v].x);
					resultMesh.vertices.push_back(mesh->mTextureCoords[0][v].y);
				}
				// Z component for texture index
				resultMesh.vertices.push_back(static_cast<float>(resultMesh.textureID));

				// only for texture mapping on stone
				if (tex_type == 1) {
					// Tangents
					resultMesh.vertices.push_back(tangents[v].x);
					resultMesh.vertices.push_back(tangents[v].y);
					resultMesh.vertices.push_back(tangents[v].z);

					// Bitangents
					resultMesh.vertices.push_back(bitangents[v].x);
					resultMesh.vertices.push_back(bitangents[v].y);
					resultMesh.vertices.push_back(bitangents[v].z);
				}


			}

			// Push indices
			for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
				aiFace face = mesh->mFaces[f];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					resultMesh.indices.push_back(face.mIndices[j]);
				}
			}
			resultMesh.count = resultMesh.indices.size();

			// VAO
			glGenVertexArrays(1, &resultMesh.VAO);
			glBindVertexArray(resultMesh.VAO);

			// VBO
			glGenBuffers(1, &resultMesh.VBO);
			glBindBuffer(GL_ARRAY_BUFFER, resultMesh.VBO);
			glBufferData(GL_ARRAY_BUFFER, resultMesh.vertices.size() * sizeof(float), resultMesh.vertices.data(), GL_STATIC_DRAW);

			// EBO
			glGenBuffers(1, &resultMesh.EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resultMesh.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, resultMesh.indices.size() * sizeof(unsigned int), resultMesh.indices.data(), GL_STATIC_DRAW);

			// Vertex attributes
			// stone needs tangent and bitangent for nomral mapping
			if (tex_type == 1) {
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);

				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(6 * sizeof(float)));
				glEnableVertexAttribArray(2);

				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(9 * sizeof(float)));
				glEnableVertexAttribArray(3);

				glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(12 * sizeof(float)));
				glEnableVertexAttribArray(4);
			}
			else {
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);

				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
				glEnableVertexAttribArray(2);
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			meshProcessed = true; // Mark mesh as processed
			return;
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene, meshProcessed);
		}
		};

	bool meshProcessed = false;
	processNode(scene->mRootNode, scene, meshProcessed);

	return resultMesh;
}

// for loading in grass/building models
Mesh loadInstanceModel(const std::string& path, int tex_type) {
	Mesh resultMesh;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		//std::cerr << "Error: " << importer.GetErrorString() << std::endl;
		return resultMesh; // Return an empty Mesh object
	}

	// Process the first node with a mesh
	std::function<void(aiNode*, const aiScene*, bool&)> processNode = [&](aiNode* node, const aiScene* scene, bool& meshProcessed) {
		if (meshProcessed) return; // Stop if we've already processed a mesh

		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			std::vector<vec3> normals(mesh->mNumVertices, vec3(0.0f, 0.0f, 0.0f));

			// Calculate normals
			for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
				aiFace face = mesh->mFaces[f];
				if (face.mNumIndices == 3) { // Ensure the face is a triangle
					aiVector3D v0 = mesh->mVertices[face.mIndices[0]];
					aiVector3D v1 = mesh->mVertices[face.mIndices[1]];
					aiVector3D v2 = mesh->mVertices[face.mIndices[2]];

					vec3 edge1 = vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
					vec3 edge2 = vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

					vec3 faceNormal = normalize(cross(edge1, edge2));

					normals[face.mIndices[0]] += faceNormal;
					normals[face.mIndices[1]] += faceNormal;
					normals[face.mIndices[2]] += faceNormal;
				}
			}

			// Normalize normals
			for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
				normals[v] = normalize(normals[v]);
			}

			//// load specific textures
			//string texturePath;
			//if (tex_type == 2) {		// grassB
			//	texturePath = "assets/grassB_albedo.png";
			//}
			//else if (tex_type == 3) {	// bush01
			//	texturePath = "assets/bush01.png";
			//}
			//else if (tex_type == 4) {	// bush05
			//	texturePath = "assets/bush05.png";
			//}
			//else if (tex_type == 5) {	// building1
			//	texturePath = "assets/Medieval_Building_LowPoly/Medieval_Building_LowPoly_V1_Albedo_small.png";
			//}
			//else if (tex_type == 6) {	// building2
			//	texturePath = "assets/Medieval_Building_LowPoly/Medieval_Building_LowPoly_V2_Albedo_small.png";
			//}

			//resultMesh.textureID = generateOpenGLTexture(texturePath.c_str(), resultMesh.textureID);
			resultMesh.textureID = tex_type;
			cout << "Instance object textureID: " << resultMesh.textureID << endl;

			for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
				// Vertices
				resultMesh.vertices.push_back(mesh->mVertices[v].x);
				resultMesh.vertices.push_back(mesh->mVertices[v].y);
				resultMesh.vertices.push_back(mesh->mVertices[v].z);

				// Normals
				resultMesh.vertices.push_back(normals[v].x);
				resultMesh.vertices.push_back(normals[v].y);
				resultMesh.vertices.push_back(normals[v].z);

				// Texture coordinates
				if (mesh->mTextureCoords[0]) {
					resultMesh.vertices.push_back(mesh->mTextureCoords[0][v].x);
					resultMesh.vertices.push_back(mesh->mTextureCoords[0][v].y);
				}

				// Z component for texture index
				resultMesh.vertices.push_back(static_cast<float>(resultMesh.textureID));
			}

			for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
				aiFace face = mesh->mFaces[f];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					resultMesh.indices.push_back(face.mIndices[j]);
				}
			}

			resultMesh.count = resultMesh.indices.size();
			/*
			// VAO
			glGenVertexArrays(1, &resultMesh.VAO);
			glBindVertexArray(resultMesh.VAO);

			// VBO
			glGenBuffers(1, &resultMesh.VBO);
			glBindBuffer(GL_ARRAY_BUFFER, resultMesh.VBO);
			glBufferData(GL_ARRAY_BUFFER, resultMesh.vertices.size() * sizeof(float), resultMesh.vertices.data(), GL_STATIC_DRAW);

			// EBO
			glGenBuffers(1, &resultMesh.EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resultMesh.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, resultMesh.indices.size() * sizeof(unsigned int), resultMesh.indices.data(), GL_STATIC_DRAW);

			// Vertex attributes
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(2);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			meshProcessed = true; // Mark mesh as processed
			*/
			return;
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene, meshProcessed);
		}
		};

	bool meshProcessed = false;
	processNode(scene->mRootNode, scene, meshProcessed);

	return resultMesh;
}

// mesh for single objects
Mesh mesh_plane, mesh_stone;

// mesh for grass/buildings
Mesh mesh_grassB, mesh_bush01, mesh_bush05;
Mesh mesh_building1, mesh_building2;

// VAO, VBO, EBO for the combined objects
GLuint all_instance_VAO, all_instance_VBO, all_instance_EBO;

// program for plane, stone
ShaderProgram* plane_ShaderProgram = new ShaderProgram();
ShaderProgram* stone_ShaderProgram = new ShaderProgram();

// uniform location for planes
GLint plane_m_loc, plane_v_loc, plane_p_loc;
GLint plane_pos_loc;

// uniform location for stone
GLint stone_m_loc, stone_v_loc, stone_p_loc;

// initialize single objects (plane, stone)
void initSingleObjects() {
	// load models (the numbers in the loadModel function are for different textures)
	////////////////////////////////////// airplane
	glActiveTexture(GL_TEXTURE4);
	mesh_plane = loadSingleModel("assets/airplane.obj", 0);

	// plane shader
	Shader* vsShader = new Shader(GL_VERTEX_SHADER);
	vsShader->createShaderFromFile("src\\shader\\planeVertexShader.glsl");
	std::cout << vsShader->shaderInfoLog() << "\n";
	Shader* fsShader = new Shader(GL_FRAGMENT_SHADER);
	fsShader->createShaderFromFile("src\\shader\\planeFragmentShader.glsl");
	std::cout << fsShader->shaderInfoLog() << "\n";
	// shader program
	plane_ShaderProgram->init();
	plane_ShaderProgram->attachShader(vsShader);
	plane_ShaderProgram->attachShader(fsShader);
	plane_ShaderProgram->checkStatus();
	if (plane_ShaderProgram->status() != ShaderProgramStatus::READY) {
		cout << "Problem with shader" << endl;
		return;
	}
	plane_ShaderProgram->linkProgram();
	plane_ShaderProgram->useProgram();

	// uniforms (MVP, plane positions)
	plane_m_loc = glGetUniformLocation(plane_ShaderProgram->programId(), "modelMat");
	plane_v_loc = glGetUniformLocation(plane_ShaderProgram->programId(), "viewMat");
	plane_p_loc = glGetUniformLocation(plane_ShaderProgram->programId(), "projMat");
	plane_pos_loc = glGetUniformLocation(plane_ShaderProgram->programId(), "planePosition");

	vsShader->releaseShader();
	fsShader->releaseShader();

	delete vsShader;
	delete fsShader;

	////////////////////////////////////// stone
	glActiveTexture(GL_TEXTURE5);
	mesh_stone = loadSingleModel("assets/MagicRock/magicRock.obj", 1);

	// stone shader
	vsShader = new Shader(GL_VERTEX_SHADER);
	vsShader->createShaderFromFile("src\\shader\\stoneVertexShader.glsl");
	std::cout << vsShader->shaderInfoLog() << "\n";
	fsShader = new Shader(GL_FRAGMENT_SHADER);
	fsShader->createShaderFromFile("src\\shader\\stoneFragmentShader.glsl");
	std::cout << fsShader->shaderInfoLog() << "\n";
	// shader program
	stone_ShaderProgram->init();
	stone_ShaderProgram->attachShader(vsShader);
	stone_ShaderProgram->attachShader(fsShader);
	stone_ShaderProgram->checkStatus();
	if (stone_ShaderProgram->status() != ShaderProgramStatus::READY) {
		cout << "Problem with shader" << endl;
		return;
	}
	stone_ShaderProgram->linkProgram();
	stone_ShaderProgram->useProgram();

	// uniforms (MVP)
	stone_v_loc = glGetUniformLocation(stone_ShaderProgram->programId(), "viewMat");
	stone_p_loc = glGetUniformLocation(stone_ShaderProgram->programId(), "projMat");

	vsShader->releaseShader();
	fsShader->releaseShader();

	delete vsShader;
	delete fsShader;
}

struct DrawCommand {
	GLuint count;
	GLuint instanceCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;
};

struct RawInstanceProperties {
	vec4 position;
	mat4 rotationMatrix;
	ivec4 indices;
};

struct InstanceProperties {
	ivec4 indices;
};

// for SSBO of instance objects
vector<DrawCommand> drawCommands(5);
vector<RawInstanceProperties> rawInstanceData(TOTAL_INSTANCE);
vector<InstanceProperties> currValidInstanceData(TOTAL_INSTANCE);
GLuint drawCommandsSSBO, rawInstanceDataSSBO, currValidInstanceDataSSBO;

// program for instance objects
ShaderProgram* reset_ShaderProgram = new ShaderProgram();
ShaderProgram* compute_ShaderProgram = new ShaderProgram();
ShaderProgram* instanceDraw_ShaderProgram = new ShaderProgram();

//// uniform location for compute shader culling
GLint compute_mv_loc, compute_p_loc;
// uniform location for instance objects
GLint instanceDraw_mv_loc, instanceDraw_p_loc;

// this function put the instane properties into rawInstanceData(TOTAL_INSTANCE)
void loadInstanceData(int start_index, int num_samples, MyPoissonSample* sample) {
	// query position and rotation
	for (int i = 0; i < num_samples; i++) {
		vec3 position(
			sample->m_positions[i * 3 + 0], sample->m_positions[i * 3 + 1], sample->m_positions[i * 3 + 2]
		);
		vec3 rad(
			sample->m_radians[i * 3 + 0], sample->m_radians[i * 3 + 1], sample->m_radians[i * 3 + 2]
		);
		// calculate rotation matrix
		quat q = quat(rad);
		mat4 rotationMatrix = mat4_cast(q);

		rawInstanceData[start_index + i].position = vec4(position, 0);
		rawInstanceData[start_index + i].rotationMatrix = rotationMatrix;
		rawInstanceData[start_index + i].indices[0] = start_index + i;
	}
	cout << "Data of instance object:" << num_samples << endl;
}

// this function initializes drawcommand for instance draw
void initDrawCommand(int index, Mesh mesh, int firstIndex, int baseInstance) {
	drawCommands[index].count = mesh.indices.size();
	drawCommands[index].instanceCount = 0;
	drawCommands[index].firstIndex = firstIndex;
	drawCommands[index].baseVertex = 0;
	drawCommands[index].baseInstance = baseInstance;
}

// initialize instance objects (grass, buldings)
// 1. load instance models and combine their data
// 2. create VAO, VBO, EBO for the combined instance objects
// 3. setup the texture array (textureArrayHandle)
// 4. load in position, rotation data of the instance objects
// 5. setup drawcommand SSBO and rawInstanceData SSBO
// 6. setup shader + programs for objects instancing (compute_resetProgram, computeProgram, instanceProgram)
void initInstanceObjects() {
	/////////// 1. oad models (the numbers in the loadModel function are for different textures)
	mesh_grassB = loadInstanceModel("assets/grassB.obj", 0);			// grassB
	mesh_bush01 = loadInstanceModel("assets/bush01_lod2.obj", 1);		// bush01
	mesh_bush05 = loadInstanceModel("assets/bush05_lod2.obj", 2);		// bush05
	mesh_building1 = loadInstanceModel("assets/Medieval_Building_LowPoly/medieval_building_lowpoly_1.obj", 3);		// building1
	mesh_building2 = loadInstanceModel("assets/Medieval_Building_LowPoly/medieval_building_lowpoly_2.obj", 4);		// building2

	// combine VAO
	// combine vertices
	vector<float> combinedVertices = mesh_grassB.vertices;
	combinedVertices.insert(combinedVertices.end(), mesh_bush01.vertices.begin(), mesh_bush01.vertices.end());
	combinedVertices.insert(combinedVertices.end(), mesh_bush05.vertices.begin(), mesh_bush05.vertices.end());
	combinedVertices.insert(combinedVertices.end(), mesh_building1.vertices.begin(), mesh_building1.vertices.end());
	combinedVertices.insert(combinedVertices.end(), mesh_building2.vertices.begin(), mesh_building2.vertices.end());

	// combine indices with offsets (can set drawCommands[].baseVertex = 0 for all mesh)
	vector<unsigned int> combinedIndices = mesh_grassB.indices;
	unsigned int VertexCount = mesh_grassB.vertices.size() / 9; // vec3(position), vec3(normal), vec3(uv)
	for (unsigned int idx : mesh_bush01.indices) {
		combinedIndices.push_back(idx + VertexCount);
	}
	VertexCount += mesh_bush01.vertices.size() / 9; // vec3(position), vec3(normal), vec3(uv)
	for (unsigned int idx : mesh_bush05.indices) {
		combinedIndices.push_back(idx + VertexCount);
	}
	VertexCount += mesh_bush05.vertices.size() / 9; // vec3(position), vec3(normal), vec3(uv)
	for (unsigned int idx : mesh_building1.indices) {
		combinedIndices.push_back(idx + VertexCount);
	}
	VertexCount += mesh_building1.vertices.size() / 9; // vec3(position), vec3(normal), vec3(uv)
	for (unsigned int idx : mesh_building2.indices) {
		combinedIndices.push_back(idx + VertexCount);
	}

	/////////// 2. create VAO, VBO, EBO for the combined instance objects
	glGenVertexArrays(1, &all_instance_VAO);
	glGenBuffers(1, &all_instance_VBO);
	glGenBuffers(1, &all_instance_EBO);

	glBindVertexArray(all_instance_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, all_instance_VBO);
	glBufferData(GL_ARRAY_BUFFER, combinedVertices.size() * sizeof(float), combinedVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, all_instance_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, combinedIndices.size() * sizeof(unsigned int), combinedIndices.data(), GL_STATIC_DRAW);

	// set vertex attribute pointers
	// vertex position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// vertex texture coordinates (u, v), texture index (z component)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	/////////// 3. load textures
	unsigned char* textureArrayData = new unsigned char[IMG_WIDTH * IMG_HEIGHT * IMG_CHANNEL * NUM_INSTANCE_TEXTURE];
	std::vector<std::string> textureFiles = {
		"assets/grassB_albedo.png",
		"assets/bush01.png",
		"assets/bush05.png",
		"assets/Medieval_Building_LowPoly/Medieval_Building_LowPoly_V1_Albedo_small.png",
		"assets/Medieval_Building_LowPoly/Medieval_Building_LowPoly_V2_Albedo_small.png"
	};

	// load and cmbine textures
	for (int i = 0; i < NUM_INSTANCE_TEXTURE; ++i) {
		texture_data texture = load_Image(textureFiles[i].c_str());

		if (texture.data) {
			// combine
			memcpy(textureArrayData + i * IMG_WIDTH * IMG_HEIGHT * IMG_CHANNEL, texture.data, IMG_WIDTH * IMG_HEIGHT * IMG_CHANNEL);
			delete[] texture.data;
		}
	}

	glActiveTexture(GL_TEXTURE7);
	glGenTextures(1, &textureArrayHandle);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayHandle);

	GLint boundTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &boundTexture);

	// Allocate storage for the texture array (NUM_TEXTURE layers)
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 11, GL_RGBA8, IMG_WIDTH, IMG_HEIGHT, NUM_INSTANCE_TEXTURE);

	// Fill the texture array with data (for each texture slice)
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, NUM_INSTANCE_TEXTURE, GL_RGBA, GL_UNSIGNED_BYTE, textureArrayData);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Generate mipmaps for the texture array
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	/////////// 4. get position, rotation data of the instance objects
	int start_index = 0;
	loadInstanceData(start_index, NUM_SAMPLE0, sample0);
	start_index += NUM_SAMPLE0;
	loadInstanceData(start_index, NUM_SAMPLE1, sample1);
	start_index += NUM_SAMPLE1;
	loadInstanceData(start_index, NUM_SAMPLE2, sample2);
	start_index += NUM_SAMPLE2;
	loadInstanceData(start_index, NUM_SAMPLE3, sample3);
	start_index += NUM_SAMPLE3;
	loadInstanceData(start_index, NUM_SAMPLE4, sample4);

	/////////// 5. setup drawcommand SSBO and rawInstanceData SSBO
	//SSBO
	glGenBuffers(1, &drawCommandsSSBO);
	glGenBuffers(1, &rawInstanceDataSSBO);
	glGenBuffers(1, &currValidInstanceDataSSBO);

	// initialize draw commands
	int first_index = 0;
	initDrawCommand(0, mesh_grassB, first_index, 0);  // command index, mesh, baseInstance
	first_index += mesh_grassB.indices.size();
	initDrawCommand(1, mesh_bush01, first_index, NUM_SAMPLE0);
	first_index += mesh_bush01.indices.size();
	initDrawCommand(2, mesh_bush05, first_index, NUM_SAMPLE0 + NUM_SAMPLE1);
	first_index += mesh_bush05.indices.size();
	initDrawCommand(3, mesh_building1, first_index, NUM_SAMPLE0 + NUM_SAMPLE1 + NUM_SAMPLE2);
	first_index += mesh_building1.indices.size();
	initDrawCommand(4, mesh_building2, first_index, NUM_SAMPLE0 + NUM_SAMPLE1 + NUM_SAMPLE2 + NUM_SAMPLE3);

	// Bind and allocate for draw commands
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawCommandsSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, drawCommands.size() * sizeof(DrawCommand), drawCommands.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, drawCommandsSSBO);

	// Bind and allocate for raw instance data
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, rawInstanceDataSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, rawInstanceData.size() * sizeof(RawInstanceProperties), rawInstanceData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, rawInstanceDataSSBO);

	// Bind and allocate for current valid instance data
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, currValidInstanceDataSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, currValidInstanceData.size() * sizeof(InstanceProperties), currValidInstanceData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, currValidInstanceDataSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	/////////// 6. setup shader + programs for objects instancing (compute_resetProgram, computeProgram, instanceProgram)
	//-------------------- reset_ShaderProgram
	Shader* resetShader = new Shader(GL_COMPUTE_SHADER);
	resetShader->createShaderFromFile("src\\shader\\resetShader.glsl");
	std::cout << resetShader->shaderInfoLog() << "\n";
	// shader program
	reset_ShaderProgram->init();
	reset_ShaderProgram->attachShader(resetShader);
	reset_ShaderProgram->checkStatus();
	if (reset_ShaderProgram->status() != ShaderProgramStatus::READY) {
		cout << "Problem with shader" << endl;
		return;
	}
	reset_ShaderProgram->linkProgram();
	reset_ShaderProgram->useProgram();

	resetShader->releaseShader();
	delete resetShader;

	//-------------------- compute_ShaderProgram
	Shader* computeShader = new Shader(GL_COMPUTE_SHADER);
	computeShader->createShaderFromFile("src\\shader\\computeShader.glsl");
	std::cout << computeShader->shaderInfoLog() << "\n";
	// shader program
	compute_ShaderProgram->init();
	compute_ShaderProgram->attachShader(computeShader);
	compute_ShaderProgram->checkStatus();
	if (compute_ShaderProgram->status() != ShaderProgramStatus::READY) {
		cout << "Problem with shader" << endl;
		return;
	}
	compute_ShaderProgram->linkProgram();
	compute_ShaderProgram->useProgram();

	computeShader->releaseShader();
	delete computeShader;

	compute_ShaderProgram->useProgram();
	// uniforms (MVP)
	compute_mv_loc = glGetUniformLocation(compute_ShaderProgram->programId(), "um4mv");
	compute_p_loc = glGetUniformLocation(compute_ShaderProgram->programId(), "um4p");

	//-------------------- instanceDraw_ShaderProgram
	Shader* vsShader = new Shader(GL_VERTEX_SHADER);
	vsShader->createShaderFromFile("src\\shader\\instanceDrawVertexShader.glsl");
	cout << vsShader->shaderInfoLog() << "\n";
	Shader* fsShader = new Shader(GL_FRAGMENT_SHADER);
	fsShader->createShaderFromFile("src\\shader\\instanceDrawFragmentShader.glsl");
	cout << fsShader->shaderInfoLog() << "\n";
	// shader program
	instanceDraw_ShaderProgram->init();
	instanceDraw_ShaderProgram->attachShader(vsShader);
	instanceDraw_ShaderProgram->attachShader(fsShader);
	instanceDraw_ShaderProgram->checkStatus();
	if (instanceDraw_ShaderProgram->status() != ShaderProgramStatus::READY) {
		cout << "Problem with shader" << endl;
		return;
	}
	instanceDraw_ShaderProgram->linkProgram();
	instanceDraw_ShaderProgram->useProgram();

	instanceDraw_ShaderProgram->useProgram();
	// uniforms (MVP)
	instanceDraw_mv_loc = glGetUniformLocation(instanceDraw_ShaderProgram->programId(), "um4mv");
	instanceDraw_p_loc = glGetUniformLocation(instanceDraw_ShaderProgram->programId(), "um4p");
	glUniform1i(glGetUniformLocation(instanceDraw_ShaderProgram->programId(), "albedoTextureArray"), 7);

	vsShader->releaseShader();
	fsShader->releaseShader();
	delete vsShader;
	delete fsShader;

}

///////////////////////////////////


void updateWhenPlayerProjectionChanged(const float nearDepth, const float farDepth);
void viewFrustumMultiClipCorner(const std::vector<float>& depths, const glm::mat4& viewMat, const glm::mat4& projMat, float* clipCorner);

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "rendering", nullptr, nullptr);
	if (window == nullptr) {
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
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	// disable vsync
	//glfwSwapInterval(0);

	////////////////// intitalize single objects
	initSingleObjects();
	////////////////// intitalize instance objects
	initInstanceObjects();



	// start game-loop
	vsyncDisabled(window);


	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void vsyncDisabled(GLFWwindow* window) {
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

bool initializeGL() {
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

	// initialize view frustum
	m_viewFrustumSO = new ViewFrustumSceneObject(2, SceneManager::Instance()->m_fs_pixelProcessIdHandle, SceneManager::Instance()->m_fs_pureColor);
	defaultRenderer->appendDynamicSceneObject(m_viewFrustumSO->sceneObject());

	// initialize terrain
	m_terrain = new MyTerrain();
	m_terrain->init(-1);
	defaultRenderer->appendTerrainSceneObject(m_terrain->sceneObject());
	// =================================================================	

	resize(FRAME_WIDTH, FRAME_HEIGHT);

	m_imguiPanel = new MyImGuiPanel();

	return true;
}
void resizeGL(GLFWwindow* window, int w, int h) {
	resize(w, h);
}

// work group numbers for compute shaders
int numWorkGroups = (TOTAL_INSTANCE + 1023) / 1024;

void paintGL() {
	// update cameras and airplane
	// god camera
	m_myCameraManager->updateGodCamera();
	// player camera
	m_myCameraManager->updatePlayerCamera();
	const glm::vec3 PLAYER_CAMERA_POSITION = m_myCameraManager->playerViewOrig();
	m_myCameraManager->adjustPlayerCameraHeight(m_terrain->terrainData()->height(PLAYER_CAMERA_POSITION.x, PLAYER_CAMERA_POSITION.z));
	// airplane
	m_myCameraManager->updateAirplane();
	const glm::vec3 AIRPLANE_POSTION = m_myCameraManager->airplanePosition();
	m_myCameraManager->adjustAirplaneHeight(m_terrain->terrainData()->height(AIRPLANE_POSTION.x, AIRPLANE_POSTION.z));

	// prepare parameters
	const glm::mat4 playerVM = m_myCameraManager->playerViewMatrix();
	const glm::mat4 playerProjMat = m_myCameraManager->playerProjectionMatrix();
	const glm::vec3 playerViewOrg = m_myCameraManager->playerViewOrig();

	const glm::mat4 godVM = m_myCameraManager->godViewMatrix();
	const glm::mat4 godProjMat = m_myCameraManager->godProjectionMatrix();

	const glm::mat4 airplaneModelMat = m_myCameraManager->airplaneModelMatrix();

	// (x, y, w, h)
	const glm::ivec4 playerViewport = m_myCameraManager->playerViewport();

	// (x, y, w, h)
	const glm::ivec4 godViewport = m_myCameraManager->godViewport();

	// ====================================================================================
	// update player camera view frustum
	m_viewFrustumSO->updateState(playerVM, playerViewOrg);

	// update geography
	m_terrain->updateState(playerVM, playerViewOrg, playerProjMat, nullptr);
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

	// rendering with god view
	defaultRenderer->setViewport(godViewport[0], godViewport[1], godViewport[2], godViewport[3]);
	defaultRenderer->setView(godVM);
	defaultRenderer->setProjection(godProjMat);
	defaultRenderer->renderPass();
	// ===============================


	// =============================== ------------------------- plane
	plane_ShaderProgram->useProgram();
	glBindVertexArray(mesh_plane.VAO);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mesh_plane.textureID);

	// player view
	defaultRenderer->setViewport(playerViewport[0], playerViewport[1], playerViewport[2], playerViewport[3]);
	glUniformMatrix4fv(plane_m_loc, 1, GL_FALSE, value_ptr(airplaneModelMat));
	glUniformMatrix4fv(plane_v_loc, 1, GL_FALSE, value_ptr(playerVM));
	glUniformMatrix4fv(plane_p_loc, 1, GL_FALSE, value_ptr(playerProjMat));
	glUniformMatrix4fv(plane_pos_loc, 1, GL_FALSE, value_ptr(AIRPLANE_POSTION));
	glDrawElements(GL_TRIANGLES, mesh_plane.indices.size(), GL_UNSIGNED_INT, 0);

	// god view
	defaultRenderer->setViewport(godViewport[0], godViewport[1], godViewport[2], godViewport[3]);
	glUniformMatrix4fv(plane_v_loc, 1, GL_FALSE, value_ptr(godVM));
	glUniformMatrix4fv(plane_p_loc, 1, GL_FALSE, value_ptr(godProjMat));
	glDrawElements(GL_TRIANGLES, mesh_plane.indices.size(), GL_UNSIGNED_INT, 0);

	// =============================== ------------------------- stone
	stone_ShaderProgram->useProgram();
	glBindVertexArray(mesh_stone.VAO);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, mesh_stone.textureID);

	// player view
	defaultRenderer->setViewport(playerViewport[0], playerViewport[1], playerViewport[2], playerViewport[3]);
	glUniformMatrix4fv(stone_v_loc, 1, GL_FALSE, value_ptr(playerVM));
	glUniformMatrix4fv(stone_p_loc, 1, GL_FALSE, value_ptr(playerProjMat));
	glDrawElements(GL_TRIANGLES, mesh_stone.indices.size(), GL_UNSIGNED_INT, 0);

	// god view
	defaultRenderer->setViewport(godViewport[0], godViewport[1], godViewport[2], godViewport[3]);
	glUniformMatrix4fv(stone_v_loc, 1, GL_FALSE, value_ptr(godVM));
	glUniformMatrix4fv(stone_p_loc, 1, GL_FALSE, value_ptr(godProjMat));
	glDrawElements(GL_TRIANGLES, mesh_stone.indices.size(), GL_UNSIGNED_INT, 0);

	// =============================== ------------------------- instance objects
	// ------rest_Program
	reset_ShaderProgram->useProgram();
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// ------compute_Program
	compute_ShaderProgram->useProgram();
	glUniformMatrix4fv(compute_mv_loc, 1, GL_FALSE, value_ptr(playerVM));
	glUniformMatrix4fv(compute_p_loc, 1, GL_FALSE, value_ptr(playerProjMat));

	glDispatchCompute(numWorkGroups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// ------instanceDraw_Program
	instanceDraw_ShaderProgram->useProgram();	// use the program that draws instance objects
	glBindVertexArray(all_instance_VAO);		// bind VAO for the combined instance

	// player view
	defaultRenderer->setViewport(playerViewport[0], playerViewport[1], playerViewport[2], playerViewport[3]);
	glUniformMatrix4fv(instanceDraw_mv_loc, 1, GL_FALSE, value_ptr(playerVM));
	glUniformMatrix4fv(instanceDraw_p_loc, 1, GL_FALSE, value_ptr(playerProjMat));
	// raw foliage using indirect drawing
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandsSSBO);    // Bind the draw commands SSBO for indirect drawing
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayHandle);

	// draw with commands
	glMultiDrawElementsIndirect(
		GL_TRIANGLES,
		GL_UNSIGNED_INT,
		0,
		drawCommands.size(),
		sizeof(DrawCommand)
	);

	// god view
	defaultRenderer->setViewport(godViewport[0], godViewport[1], godViewport[2], godViewport[3]);
	glUniformMatrix4fv(instanceDraw_mv_loc, 1, GL_FALSE, value_ptr(godVM));
	glUniformMatrix4fv(instanceDraw_p_loc, 1, GL_FALSE, value_ptr(godProjMat));

	// draw with commands
	glMultiDrawElementsIndirect(
		GL_TRIANGLES,
		GL_UNSIGNED_INT,
		0,
		drawCommands.size(),
		sizeof(DrawCommand)
	);


	//cout << drawCommands[1].instanceCount << endl;


	ImGui::Begin("My name is window");
	m_imguiPanel->update();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

////////////////////////////////////////////////
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
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
void cursorPosCallback(GLFWwindow* window, double x, double y) {
	cursorPos[0] = x;
	cursorPos[1] = y;

	m_myCameraManager->mouseMove(x, y);
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {}

void updateWhenPlayerProjectionChanged(const float nearDepth, const float farDepth) {
	// get view frustum corner
	const int NUM_CASCADE = 2;
	const float HY = 0.0;

	float dOffset = (farDepth - nearDepth) / NUM_CASCADE;
	float* corners = new float[(NUM_CASCADE + 1) * 12];
	std::vector<float> depths(NUM_CASCADE + 1);
	for (int i = 0; i < NUM_CASCADE; i++) {
		depths[i] = nearDepth + dOffset * i;
	}
	depths[NUM_CASCADE] = farDepth;
	// get viewspace corners
	glm::mat4 tView = glm::lookAt(glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	// calculate corners of view frustum cascade
	viewFrustumMultiClipCorner(depths, tView, m_myCameraManager->playerProjectionMatrix(), corners);

	// update view frustum scene object
	for (int i = 0; i < NUM_CASCADE + 1; i++) {
		float* layerBuffer = m_viewFrustumSO->cascadeDataBuffer(i);
		for (int j = 0; j < 12; j++) {
			layerBuffer[j] = corners[i * 12 + j];
		}
	}
	m_viewFrustumSO->updateDataBuffer();

	delete corners;
}
void resize(const int w, const int h) {
	FRAME_WIDTH = w;
	FRAME_HEIGHT = h;

	m_myCameraManager->resize(w, h);
	defaultRenderer->resize(w, h);
	updateWhenPlayerProjectionChanged(0.1, m_myCameraManager->playerCameraFar());
}
void viewFrustumMultiClipCorner(const std::vector<float>& depths, const glm::mat4& viewMat, const glm::mat4& projMat, float* clipCorner) {
	const int NUM_CLIP = depths.size();

	// Calculate Inverse
	glm::mat4 viewProjInv = glm::inverse(projMat * viewMat);

	// Calculate Clip Plane Corners
	int clipOffset = 0;
	for (const float depth : depths) {
		// Get Depth in NDC, the depth in viewSpace is negative
		glm::vec4 v = glm::vec4(0, 0, -1 * depth, 1);
		glm::vec4 vInNDC = projMat * v;
		if (fabs(vInNDC.w) > 0.00001) {
			vInNDC.z = vInNDC.z / vInNDC.w;
		}
		// Get 4 corner of clip plane
		float cornerXY[] = {
			-1, 1,
			-1, -1,
			1, -1,
			1, 1
		};
		for (int j = 0; j < 4; j++) {
			glm::vec4 wcc = {
				cornerXY[j * 2 + 0], cornerXY[j * 2 + 1], vInNDC.z, 1
			};
			wcc = viewProjInv * wcc;
			wcc = wcc / wcc.w;

			clipCorner[clipOffset * 12 + j * 3 + 0] = wcc[0];
			clipCorner[clipOffset * 12 + j * 3 + 1] = wcc[1];
			clipCorner[clipOffset * 12 + j * 3 + 2] = wcc[2];
		}
		clipOffset = clipOffset + 1;
	}
}
