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
	void post_process_update();

	bool deferred_init();
	bool shadowmap_init();
	bool post_process_init();
	bool ssao_init();
public:
	

private:
	int m_window_width;
	int m_window_height;
	//GLuint m_normalMapHandle;
	//GLuint m_albedoMapHandle;

	vector<MyMesh> m_roomMeshes;
	vector<MyMesh> m_triceMeshes;
	vector<MyMesh> m_sphere;

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

	// post-process programs
	ShaderProgram* m_bloomProgram;
	ShaderProgram* m_blurProgram;
	ShaderProgram* m_fxaaProgram;
	ShaderProgram* m_volumetricProgram;

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
	// post-process program variables
	struct
	{
		GLuint fbo;
		GLuint scene;
		GLuint bright_scene;
	} post_process_buffer;

	struct
	{
		GLuint fbo[2];
		GLuint buffer[2];
		bool now = true;
		bool first = true;
	} pingpong;
	
	vec2 screen_light_pos() {
		vec4 clip_space_pos = m_projMat * m_viewMat * vec4(light_sphere.volumetricLightPos, 1.0);
		vec3 ndc_space_pos = vec3(clip_space_pos) / clip_space_pos.w;

		vec2 screen_light_pos;
		screen_light_pos.x = 0.5f * (ndc_space_pos.x + 1.0f);
		screen_light_pos.y = 0.5f * (ndc_space_pos.y + 1.0f);

		return screen_light_pos;
	}
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
	// geometry of some light source, 
	// the transformation will be set up from the parameter in scenemanager
	struct {
		GLuint vao;
		GLuint vbo_position;
		float height;
		float width;
		vec3 center;
		vec3 color;
		
		mat4 model_mat() {
			vec3 translate_vector = SceneManager::Instance()->renderFeature.areaLight.lightPos;
			vec2 rotate_vector = SceneManager::Instance()->renderFeature.areaLight.lightRotate;
			mat4 matrix = translate(mat4(1.0f), translate_vector) * 
						  rotate(mat4(1.0), rotate_vector.y, vec3(0.0 ,1.0 ,0.0)) * 
						  rotate(mat4(1.0), rotate_vector.x, vec3(1.0 ,0.0 ,0.0)) * 
						  translate(mat4(1.0f), - center);
			return matrix;
		}

		// corner[0]: left upper
		// corner[1]: left bottom
		// corner[2]: right bottom
		// corner[3]: right upper
		mat4 corner() {
			float top = center.y + height * 0.5f;
			float bottom = center.y - height * 0.5f;
			float right = center.x + width * 0.5f;
			float left = center.x - width * 0.5f;

			vec4 col0 = vec4( vec3(model_mat() * vec4(left, top, center.z, 1.0)), 1.0);
			vec4 col1 = vec4( vec3(model_mat() * vec4(left, bottom, center.z, 1.0)), 1.0);
			vec4 col2 = vec4( vec3(model_mat() * vec4(right, bottom, center.z, 1.0)), 1.0);
			vec4 col3 = vec4( vec3(model_mat() * vec4(right, top, center.z, 1.0)), 1.0);

			return mat4(col0, col1, col2, col3);
		}
	}areaLight_rect; 

	struct {
		vector<MyMesh> sphere_mesh;
		vec3 pointLightPos;
		vec3 volumetricLightPos;

		mat4 model_mat(int lightType) { // 0: pointLight, 1: volumetricLight
			pointLightPos = SceneManager::Instance()->renderFeature.pointLight.lightPos;
			volumetricLightPos = SceneManager::Instance()->renderFeature.blinnPhongLight.lightPos * vec3(5, 2.5, 5);

			vec3 translate_vector;
			vec3 scale_vector = vec3(1.0);

			if (lightType == 0) {
				translate_vector = pointLightPos;
				scale_vector = vec3(0.22);
			}
			else if (lightType == 1) translate_vector = volumetricLightPos;

			mat4 matrix = translate(mat4(1.0f), translate_vector) * scale(mat4(1.0f), scale_vector);
			return matrix;
		}
	}light_sphere;

	struct {
		GLuint uboSSAOKernal;
		GLuint noiseMap;
	}ssao;
};

