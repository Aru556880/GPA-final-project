#include "IndoorSceneObject.h"

#include <iostream>
#include <string>


using std::cout;
using std::string;

string indoor_filepath = "assets/indoor/Grey_White_Room_new.obj";
string indoor_filepath2 = "assets/indoor/Grey_White_Room_old.obj";

void LoadModel(vector<Shape>& , string, uint, uint);

IndoorSceneObject::IndoorSceneObject()
{
	
}


IndoorSceneObject::~IndoorSceneObject()
{

}

void IndoorSceneObject::init() {
	LoadModel(this->m_shapes, indoor_filepath, 0, 21);
	LoadModel(this->m_shapes, indoor_filepath2, 65, 68);
}

void IndoorSceneObject::update() {
	for(int i=0;i<this->m_shapes.size();++i) {
		//if (i > SceneManager::Instance()->m_test_meshcount)
		//	break;

		glBindVertexArray(this->m_shapes[i].vao);
		bool useTexture = this->m_shapes[i].material.useTextrue;
		if (useTexture) {
			glBindTexture(GL_TEXTURE_2D, this->m_shapes[i].material.diffuse_tex);
			glUniform1i(SceneManager::Instance()->m_fs_useTexture, 1);
		}
		else
		{
			glUniform1i(SceneManager::Instance()->m_fs_useTexture, -1);
		}
			

		glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(this->m_modelMat));
		glUniform1i(SceneManager::Instance()->m_fs_pixelProcessIdHandle, this->m_pixelFunctionId);
		glUniform3fv(SceneManager::Instance()->m_fs_diffuseAlbedo, 1, value_ptr(this->m_shapes[i].material.Kd));
		glDrawElements(GL_TRIANGLES, this->m_shapes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}
}


void LoadModel(vector<Shape>& shapes ,string filePath, uint start_mesh, uint end_mesh) {
	const aiScene* scene;

	scene = aiImportFile(filePath.c_str(), aiProcess_Triangulate);

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
	for (unsigned int i = 0; i < scene->mNumMaterials; ++i){
		Material Material;
		aiMaterial* material = scene->mMaterials[i];
		aiString texturePath;
		aiColor3D color;

		// diffuse texture
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
		{
			string imgPath = "assets/indoor/" + string(texturePath.C_Str());
			texture_data tdata = loadImg(imgPath.c_str());
			glGenTextures(1, &Material.diffuse_tex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Material.diffuse_tex);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);

			glGenerateMipmap(GL_TEXTURE_2D);

			Material.useTextrue = true;
		}
		else
		{
			Material.diffuse_tex = 0;
			Material.useTextrue = false;
		}

		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		Material.Kd = vec3(color.r, color.g, color.b);
		materials.push_back(Material);
	}
	cout << "finish material loading!" << endl;

	// load geometry and save them with material
	int meshnum = glm::min(scene->mNumMeshes, end_mesh);
	for (unsigned int i = start_mesh; i < meshnum; ++i) {
		aiMesh* mesh = scene->mMeshes[i];

		Shape shape;
		glGenVertexArrays(1, &shape.vao);

		vector<float> vertices, texcoords, normals;
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

		int materialID = mesh->mMaterialIndex;
		shape.drawCount = mesh->mNumFaces * 3;
		shape.material = materials[materialID];
		shapes.push_back(shape);
	}

	aiReleaseImport(scene);
	glBindVertexArray(0);
}