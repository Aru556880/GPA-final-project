#include "IndoorSceneObject.h"

#include <iostream>
#include <string>


using std::cout;
using std::string;

string indoor_filepath = "assets/indoor/models/Grey_White_Room.obj";

void LoadModel(vector<Shape>& , string);

IndoorSceneObject::IndoorSceneObject()
{
	
}


IndoorSceneObject::~IndoorSceneObject()
{

}

void IndoorSceneObject::init() {
	LoadModel(this->m_shapes, indoor_filepath);
}

void IndoorSceneObject::update() {
	for(int i=0;i<this->m_shapes.size();++i) {

		glBindVertexArray(this->m_shapes[i].vao);
		glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(this->m_modelMat));

		glUniform1i(SceneManager::Instance()->m_fs_pixelProcessIdHandle, this->m_pixelFunctionId);
		glDrawElements(GL_TRIANGLES, this->m_shapes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}
}


void LoadModel(vector<Shape>& shapes ,string filePath) {
	const aiScene* scene;

	scene = aiImportFile(filePath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

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

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[i];
		Shape shape;

		
		glGenVertexArrays(1, &shape.vao);

		vector<float> vertices, texcoords, normals;
		vector<unsigned int> indices;

		for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
		{
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
		}

		// create 1 ibo to hold data
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
		{
			indices.push_back(mesh->mFaces[f].mIndices[0]);
			indices.push_back(mesh->mFaces[f].mIndices[1]);
			indices.push_back(mesh->mFaces[f].mIndices[2]);
		}

		glBindVertexArray(shape.vao);

		glGenBuffers(1, &shape.vbo_position);
		glGenBuffers(1, &shape.vbo_normal);
		glGenBuffers(1, &shape.vbo_texcoord);
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
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GL_UNSIGNED_INT), indices.data(), GL_STATIC_DRAW);

		shape.materialID = mesh->mMaterialIndex;
		shape.drawCount = mesh->mNumFaces * 3;

		shapes.push_back(shape);

	}

	aiReleaseImport(scene);
	glBindVertexArray(0);
}