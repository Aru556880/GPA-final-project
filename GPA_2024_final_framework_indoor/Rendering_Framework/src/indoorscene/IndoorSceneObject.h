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
	// if no geometry shader, just pass an empty string
	bool setShaderProgram(ShaderProgram*& shaderProgram,string vs,string gs, string fs);
	void setUniforms();

	void deferred_update();
	void shadowmap_update();
	void shadowmap_draw(); //discarded now, shadowmap is now in deferred shader

	bool deferred_init();
	bool shadowmap_init();
public:
	

private:
	int m_window_width;
	int m_window_height;
	//GLuint m_normalMapHandle;
	//GLuint m_albedoMapHandle;

	vector<MyMesh> m_roomMeshes;
	vector<MyMesh> m_triceMeshes;

	mat4 m_modelMat = mat4(1.0);
	mat4 m_projMat = mat4(1.0);
	mat4 m_viewMat = mat4(1.0);
	mat4 m_shadow_sbpv_matrix = mat4(1.0);

	// deferred programs
	ShaderProgram* m_geometryProgram; // calculate geometry properties, don't confuse with geometry shader(xxx_gs)
	ShaderProgram* m_deferredProgram;

	// shadow mapping programs
	ShaderProgram* m_dirLightShadowProgram;
	ShaderProgram* m_pointLightShadowProgram;

	ShaderProgram* m_blinnphongProgram;

	// deferred program variables
	struct
	{
		GLuint fbo;
		GLuint position_map;
		GLuint normal_map;
		GLuint ambient_map;
		GLuint diffuse_map;
		GLuint specular_map;
		GLuint tangent_map;
		GLuint normalTex_map; // texture of normal mapping, set vec3(0) if not used

		GLuint depth_map;
		GLuint vao;
	} gbuffer;
	// ===============================

	// directional shadow mapping program variables
	struct {

		mat4 light_view_matrix;
		mat4 light_proj_matrix;

		GLint light_mvp;

		GLuint depth_fbo;
		GLuint depth_tex;
	
	}dirLight_shadowmap;

	// ===============================
	struct {
		GLfloat far_plane;
		GLfloat near_plane;
		GLuint depth_fbo;
		GLuint depth_cubeMap;

	}pointLight_shadowmap;

	// ==============================
	struct {
		GLuint vao;
		GLuint vbo_position;
		float height;
		float width;
		vec3 center;
		vec3 color;
		mat4 model_mat;
	}area_light; 

};

