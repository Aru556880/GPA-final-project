#include "IndoorSceneObject.h"

#include <iostream>
#include <string>
#define SHADOW_MAP_SIZE 1024

GLuint test;
using std::cout;
using std::string;

string indoor_filepath = "assets/indoor/Grey_White_Room_new.obj";
string indoor_filepath2 = "assets/indoor/Grey_White_Room_old.obj";

void LoadModel(vector<MyMesh>& , string, uint, uint);

IndoorSceneObject::IndoorSceneObject()
{
	
}


IndoorSceneObject::~IndoorSceneObject()
{

}
void IndoorSceneObject::setUniforms() {

}
bool IndoorSceneObject::init() {
	bool flag = true;
	setUniforms();

	flag &= deferred_init();
	flag &= shadowmap_init();

	// load wood table meshes and other meshes separately
	LoadModel(this->m_meshes, indoor_filepath, 0, 21);
	LoadModel(this->m_meshes, indoor_filepath2, 65, 68);

	
	return flag;
}
/////////////////////////////////////////////////////////
bool IndoorSceneObject::deferred_init(){
	bool flag = true;
	flag &= setShaderProgram(m_deferredProgram, "deferred_vs.glsl", "deferred_fs.glsl");
	flag &= setShaderProgram(m_geometryProgram, "geometry_vs.glsl", "geometry_fs.glsl");
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// initial geometry buffer
	glGenFramebuffers(1, &gbuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.fbo);

	glGenTextures(4, &gbuffer.position_map);

	glBindTexture(GL_TEXTURE_2D, gbuffer.position_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.normal_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.diffuse_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gbuffer.position_map, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gbuffer.normal_map, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, gbuffer.diffuse_map, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gbuffer.depth_map, 0);

	glGenVertexArrays(1, &gbuffer.vao);
	glBindVertexArray(gbuffer.vao);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return flag;
}

bool IndoorSceneObject::shadowmap_init()
{
	bool flag = true;

	flag &= setShaderProgram(m_depthProgram, "depth_vs.glsl", "depth_fs.glsl");
	flag &= setShaderProgram(m_blinnphongProgram, "blinnphong_vs.glsl", "blinnphong_fs.glsl");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.2, 0.2, 0.2, 1.0);

	glGenFramebuffers(1, &shadowmap.depth_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowmap.depth_fbo);

	glGenTextures(1, &shadowmap.depth_tex);
	glBindTexture(GL_TEXTURE_2D, shadowmap.depth_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmap.depth_tex, 0);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return flag;
}

bool IndoorSceneObject::setShaderProgram(ShaderProgram*& m_shaderProgram, string vs, string fs)
{
	// initialize shader program
	// vertex shader
	cout << "Shader: " + vs << endl;
	Shader* vsShader = new Shader(GL_VERTEX_SHADER);
	vsShader->createShaderFromFile("src\\shader\\" + vs);
	std::cout << vsShader->shaderInfoLog() << "\n";

	// fragment shader
	cout << "Shader: " + fs << endl;
	Shader* fsShader = new Shader(GL_FRAGMENT_SHADER);
	fsShader->createShaderFromFile("src\\shader\\" + fs);
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
	m_shaderProgram = shaderProgram;

	vsShader->releaseShader();
	fsShader->releaseShader();

	delete vsShader;
	delete fsShader;

	return true;
}

void IndoorSceneObject::resize(int w, int h) {
	this->m_window_width = w;
	this->m_window_height = h;

	glBindTexture(GL_TEXTURE_2D, gbuffer.position_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, gbuffer.normal_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, gbuffer.diffuse_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, gbuffer.depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

}

void IndoorSceneObject::update(){
	this->shadowmap_update();
}

void IndoorSceneObject::default_update() {

	m_blinnphongProgram->useProgram();
	glUniformMatrix4fv(SceneManager::Instance()->m_projMatHandle, 1, false, glm::value_ptr(this->m_projMat));
	glUniformMatrix4fv(SceneManager::Instance()->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));
	glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(this->m_modelMat));

	for (int i = 0;i < m_meshes.size();++i) {
	
		glBindVertexArray(m_meshes[i].vao);

		glBindTexture(GL_TEXTURE_2D, m_meshes[i].material.diffuse_tex);

		glUniform3fv(SceneManager::Instance()->m_lightPositionHandle, 1, value_ptr(SceneManager::Instance()->light_position));
		glUniform3fv(SceneManager::Instance()->m_ambientAlbedoHandle, 1, value_ptr(m_meshes[i].material.Ka));
		glUniform3fv(SceneManager::Instance()->m_diffuseAlbedoHandle, 1, value_ptr(m_meshes[i].material.Kd));
		glUniform3fv(SceneManager::Instance()->m_specularAlbedoHandle, 1, value_ptr(m_meshes[i].material.Ks));
		glUniform1f(SceneManager::Instance()->m_shininessHandle, m_meshes[i].material.shininess);

		glDrawElements(GL_TRIANGLES, m_meshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}

}


void IndoorSceneObject::deferred_update() {
	// geometry pass
	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.fbo);
	const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, draw_buffers);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_geometryProgram->useProgram();
	glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(this->m_modelMat));
	glUniformMatrix4fv(SceneManager::Instance()->m_projMatHandle, 1, false, glm::value_ptr(this->m_projMat));
	glUniformMatrix4fv(SceneManager::Instance()->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	for (int i = 0;i < m_meshes.size();++i) {

		glBindVertexArray(m_meshes[i].vao);
		glBindTexture(GL_TEXTURE_2D, m_meshes[i].material.diffuse_tex);
		glDrawElements(GL_TRIANGLES, m_meshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}

	// deferred pass
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	m_deferredProgram->useProgram();
	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(gbuffer.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gbuffer.diffuse_map);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


}

void IndoorSceneObject::shadowmap_update(){
	
	const float shadow_range = 10.0f;
	vec3 light_position = vec3(-2.845f, 2.028f, -1.293f);
	vec3 light_lookCenter = vec3(0.542f, -0.141f, -0.422f);
	mat4 scale_bias_matrix =
		translate(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f)) * scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));

	mat4 light_proj_matrix = ortho(-shadow_range, shadow_range, -shadow_range, shadow_range, 0.0f, 50.0f);
	mat4 light_view_matrix = lookAt(light_position, light_lookCenter, vec3(0.0f, 1.0f, 0.0f));
	mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;

	mat4 shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;

	glEnable(GL_DEPTH_TEST);
	
	// shadow map pass
	m_depthProgram->useProgram();
	glBindFramebuffer(GL_FRAMEBUFFER, shadowmap.depth_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	
	shadowmap.light_mvp = 0;
	glUniformMatrix4fv(shadowmap.light_mvp, 1, GL_FALSE, value_ptr(light_vp_matrix * this->m_modelMat));
	
	for (int i = 0;i < m_meshes.size();++i) {
		glBindVertexArray(m_meshes[i].vao);
		glDrawElements(GL_TRIANGLES, m_meshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_window_width, m_window_height);
	/*
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	m_deferredProgram->useProgram();

	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(gbuffer.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowmap.depth_tex);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);*/

	// shading pass
	
	m_blinnphongProgram->useProgram();
	glUniformMatrix4fv(SceneManager::Instance()->m_projMatHandle, 1, false, glm::value_ptr(this->m_projMat));
	glUniformMatrix4fv(SceneManager::Instance()->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));
	glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(this->m_modelMat));
	
	mat4 shadow_matrix = shadow_sbpv_matrix * this->m_modelMat;

	for (int i = 0;i < m_meshes.size();++i) {

		glBindVertexArray(m_meshes[i].vao);
		
		glUniformMatrix4fv(SceneManager::Instance()->m_shadowMatHandle, 1, false, glm::value_ptr(shadow_matrix));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_meshes[i].material.diffuse_tex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowmap.depth_tex);

		glUniform3fv(SceneManager::Instance()->m_lightPositionHandle, 1, value_ptr(SceneManager::Instance()->light_position));
		glUniform3fv(SceneManager::Instance()->m_ambientAlbedoHandle, 1, value_ptr(m_meshes[i].material.Ka));
		glUniform3fv(SceneManager::Instance()->m_diffuseAlbedoHandle, 1, value_ptr(m_meshes[i].material.Kd));
		glUniform3fv(SceneManager::Instance()->m_specularAlbedoHandle, 1, value_ptr(m_meshes[i].material.Ks));
		glUniform1f(SceneManager::Instance()->m_shininessHandle, m_meshes[i].material.shininess);

		glDrawElements(GL_TRIANGLES, m_meshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}

}

namespace DEFERRED_SHADING_TYPR {
	const GLuint BLINNPHONG = 0;

	const GLint WORLD_COORD = 1;
	const GLint NORMAL = 2;
	const GLint DIFFUSE = 3;

}