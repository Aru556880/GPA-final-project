#pragma once

#include <glad\glad.h>
#include <glm\glm.hpp>

// Singleton
class SceneManager
{
private:
	SceneManager(){}
	

public:	

	virtual ~SceneManager() {}

	static SceneManager *Instance() {
		static SceneManager *m_instance = nullptr;
		if (m_instance == nullptr) {
			m_instance = new SceneManager();
		}
		return m_instance;
	}
	
	GLuint m_vertexHandle;
	GLuint m_normalHandle;
	GLuint m_uvHandle;

	GLuint m_modelMatHandle;
	GLuint m_viewMatHandle;
	GLuint m_projMatHandle;

	GLuint m_lightPositionHandle;
	GLint m_ambientAlbedoHandle;
	GLint m_diffuseAlbedoHandle;
	GLint m_specularAlbedoHandle;
	GLint m_shininessHandle;


	GLuint m_albedoMapHandle;
	GLuint m_normalMapHandle;
	
	GLuint m_fs_pixelProcessIdHandle;
	GLuint m_vs_vertexProcessIdHandle;

	GLenum m_albedoTexUnit;
	GLenum m_normalTexUnit;


	int m_albedoMapTexIdx;
	int m_normalMapTexIdx;

	int m_vs_commonProcess;
	
	int m_fs_pureColor;	
	int m_fs_terrainPass;
	int m_fs_useTexture;

	float m_test_meshcount = 0;
	glm::vec3 light_position = glm::vec3(-2.845, 2.028, -1.293);
};

