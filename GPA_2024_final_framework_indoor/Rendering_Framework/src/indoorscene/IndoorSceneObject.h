#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Rendering_Framework\src\SceneManager.h>
#include "Rendering_Framework\src\Common.h"
#include "Rendering_Framework\src\Shader.h"

class IndoorSceneObject 
{
public:
	IndoorSceneObject();
	virtual ~IndoorSceneObject();

	void update();
	bool init();
	void resize(int w, int h);
	inline void set_mvMat(mat4 viewMat, mat4 projMat) {
		m_viewMat = viewMat;
		m_projMat = projMat;
	}
private:
	bool setShaderProgram(ShaderProgram*& shaderProgram,string vs, string fs);
	void setUniforms();
	void default_update();
	void deferred_update();
	void shadowmap_update();

	bool deferred_init();
	bool shadowmap_init();
public:
	

private:
	int m_window_width;
	int m_window_height;
	//GLuint m_normalMapHandle;
	//GLuint m_albedoMapHandle;

	vector<MyMesh> m_meshes;
	mat4 m_modelMat = mat4(1.0);
	mat4 m_projMat = mat4(1.0);;
	mat4 m_viewMat = mat4(1.0);;
	

	// deferred programs
	ShaderProgram* m_geometryProgram;
	ShaderProgram* m_deferredProgram;

	// shadow mapping programs
	ShaderProgram* m_depthProgram;
	ShaderProgram* m_blinnphongProgram;

	// deferred program variables
	struct
	{
		GLuint fbo;
		GLuint position_map;
		GLuint normal_map;
		GLuint diffuse_map;
		GLuint depth_map;
		GLuint vao;
	} gbuffer;
	// ===============================

	// shadow mapping program variables
	struct {

		mat4 light_view_matrix;
		mat4 light_proj_matrix;

		GLint light_mvp;

		GLuint depth_fbo;
		GLuint depth_tex;
	
	}shadowmap;
	// ===============================



};

