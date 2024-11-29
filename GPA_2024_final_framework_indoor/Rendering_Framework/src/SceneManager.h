#pragma once

#include <glad\glad.h>
#include <glm\glm.hpp>

using namespace glm;
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

	GLuint m_shadowMatHandle;

	GLuint m_lightPositionHandle;
	GLint m_ambientAlbedoHandle;
	GLint m_diffuseAlbedoHandle;
	GLint m_specularAlbedoHandle;
	GLint m_shininessHandle;


	GLuint m_albedoMapHandle;
	GLuint m_normalMapHandle;
	GLuint m_depthMapHandle;

	GLenum m_albedoTexUnit;
	GLenum m_normalTexUnit;

	GLuint m_useNormalMapHandle;

	GLuint m_shadowMatricesHandle;
	GLuint m_farPlaneHandle;

	int m_albedoMapTexIdx;
	int m_normalMapTexIdx;


	float m_test_meshcount = 0;

	// dirLightPos = vec3(-2.845f, 2.028f, -1.293f);
	//vec3 pointLightPos = vec3(1.87659f, 0.4625f, 0.103928f);
	//vec3 areaLightPos = vec3(1.0f, 0.5f, -0.5f);

	struct {
		bool enableNormalMap = false;
		bool enableFXAA = false;

		struct {
			bool enableLight = false;
			bool enableShadow = false;
			vec3 lightPos = vec3(-2.845f, 2.028f, -1.293f);
		}blinnPhongLight;

		struct {
			bool enableLight = false;
			bool enableShadow = false;
			vec3 lightPos = vec3(1.87659f, 0.4625f, 0.103928f);
		}pointLight;

		struct {
			bool enableLight = false;
			vec2 lightRotate = vec2(0.0, 0.0);
			vec3 lightPos = vec3(1.0f, 0.5f, -0.5f);
		}areaLight;

		struct {
			bool enableDeferredMap = false;
			const char* items[5] = { "world space vertex", "world space normal", "ambient color map", "diffuse color map", "specular color map" };
			int current_item = 0;
		}deferredShading;

	}renderFeature;
};

