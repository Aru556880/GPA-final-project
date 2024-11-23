#include "IndoorSceneObject.h"

#include <iostream>
#include <string>
#define SHADOW_MAP_SIZE 4096

GLuint test;
using std::cout;
using std::string;

string room1_filepath = "assets/indoor/Grey_White_Room_new.obj";
string room2_filepath = "assets/indoor/Grey_White_Room_old.obj";
string trice_filepath = "assets/indoor/trice.obj";

void LoadMeshModel(vector<MyMesh>& , string, uint, uint);

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
	LoadMeshModel(this->m_triceMeshes, trice_filepath, 0, 1);
	LoadMeshModel(this->m_roomMeshes, room1_filepath, 0, 21);
	LoadMeshModel(this->m_roomMeshes, room2_filepath, 65, 68);

	for (int i = 0;i < this->m_roomMeshes.size();++i) {
		this->m_roomMeshes[i].m_modelMat = mat4(1.0);
	}
	
	for (int i = 0;i < this->m_triceMeshes.size();++i) {
		mat4 modelMat = 
		this->m_triceMeshes[i].m_modelMat = 
			translate(mat4(1.0f), vec3(2.05, 0.628725, -1.9)) * scale(mat4(1.0f), vec3(0.001f, 0.001f, 0.001f)) ;
	}
	glClearColor(0.2, 0.2, 0.2, 1.0);

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
	glGenTextures(8, &gbuffer.position_map);

	glBindTexture(GL_TEXTURE_2D, gbuffer.position_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.normal_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.ambient_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.diffuse_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.specular_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.tangent_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.normalTex_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, gbuffer.depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gbuffer.position_map, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gbuffer.normal_map, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, gbuffer.ambient_map, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, gbuffer.diffuse_map, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, gbuffer.specular_map, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, gbuffer.tangent_map, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, gbuffer.normalTex_map, 0);
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

	glBindTexture(GL_TEXTURE_2D, gbuffer.ambient_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, gbuffer.diffuse_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, gbuffer.specular_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, gbuffer.tangent_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, gbuffer.normalTex_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, gbuffer.depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

}

void IndoorSceneObject::update(){

	this->shadowmap_update();
	this->deferred_update();
}


void IndoorSceneObject::deferred_update() {
	// geometry pass:

	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.fbo);
	const GLenum draw_buffers[] = 
		{   GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, 
			GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
			GL_COLOR_ATTACHMENT6 };

	glDrawBuffers(7, draw_buffers);
	glClearColor(1.0,1.0,1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_geometryProgram->useProgram();

	glUniformMatrix4fv(SceneManager::Instance()->m_projMatHandle, 1, false, glm::value_ptr(this->m_projMat));
	glUniformMatrix4fv(SceneManager::Instance()->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glUniform1i(SceneManager::Instance()->m_useNormalMapHandle, 0);
	for (int i = 0;i < m_roomMeshes.size();++i) {
		glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(m_roomMeshes[i].m_modelMat));
		glBindVertexArray(m_roomMeshes[i].vao);
		glUniform3fv(SceneManager::Instance()->m_ambientAlbedoHandle, 1, value_ptr(m_roomMeshes[i].material.Ka));
		glUniform3fv(SceneManager::Instance()->m_diffuseAlbedoHandle, 1, value_ptr(m_roomMeshes[i].material.Kd));
		glUniform3fv(SceneManager::Instance()->m_specularAlbedoHandle, 1, value_ptr(m_roomMeshes[i].material.Ks));;
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_roomMeshes[i].material.diffuse_tex);

		glDrawElements(GL_TRIANGLES, m_roomMeshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}
	
	glUniform1i(SceneManager::Instance()->m_useNormalMapHandle, 1);
	for (int i = 0;i < m_triceMeshes.size();++i) {
		glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(m_triceMeshes[i].m_modelMat));
		glBindVertexArray(m_triceMeshes[i].vao);
		glUniform3fv(SceneManager::Instance()->m_ambientAlbedoHandle, 1, value_ptr(m_triceMeshes[i].material.Ka));
		glUniform3fv(SceneManager::Instance()->m_diffuseAlbedoHandle, 1, value_ptr(m_triceMeshes[i].material.Kd));
		glUniform3fv(SceneManager::Instance()->m_specularAlbedoHandle, 1, value_ptr(m_triceMeshes[i].material.Ks));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_triceMeshes[i].material.diffuse_tex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_triceMeshes[i].material.normalmap_tex);

		glDrawElements(GL_TRIANGLES, m_triceMeshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}

	// deferred pass:
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	m_deferredProgram->useProgram();
	glBindVertexArray(gbuffer.vao);

	// deferred shading type
	glUniform1i(21, 5);

	// matrix
	glUniformMatrix4fv(SceneManager::Instance()->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));
	glUniformMatrix4fv(SceneManager::Instance()->m_shadowMatHandle, 1, false, glm::value_ptr(this->m_shadow_sbpv_matrix));

	// light position
	glUniform3fv(SceneManager::Instance()->m_lightPositionHandle, 1, value_ptr(SceneManager::Instance()->light_position));

	// gbuffer texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gbuffer.position_map);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gbuffer.normal_map);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gbuffer.ambient_map);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gbuffer.diffuse_map);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gbuffer.specular_map);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gbuffer.tangent_map);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, gbuffer.normalTex_map);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, shadowmap.depth_tex);

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

	m_shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;

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
	
	for (int i = 0;i < m_roomMeshes.size();++i) {
		glUniformMatrix4fv(shadowmap.light_mvp, 1, GL_FALSE, value_ptr(light_vp_matrix * m_roomMeshes[i].m_modelMat));
		glBindVertexArray(m_roomMeshes[i].vao);
		glDrawElements(GL_TRIANGLES, m_roomMeshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}

	for (int i = 0;i < m_triceMeshes.size();++i) {
		glUniformMatrix4fv(shadowmap.light_mvp, 1, GL_FALSE, value_ptr(light_vp_matrix * m_triceMeshes[i].m_modelMat));
		glBindVertexArray(m_triceMeshes[i].vao);
		glDrawElements(GL_TRIANGLES, m_triceMeshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_window_width, m_window_height);
}

void IndoorSceneObject::shadowmap_draw() {
	m_blinnphongProgram->useProgram();
	glUniformMatrix4fv(SceneManager::Instance()->m_projMatHandle, 1, false, glm::value_ptr(this->m_projMat));
	glUniformMatrix4fv(SceneManager::Instance()->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));

	mat4 shadow_matrix = mat4(1.0);

	glUniform1i(20, -1);
	for (int i = 0;i < m_roomMeshes.size();++i) {

		glBindVertexArray(m_roomMeshes[i].vao);

		shadow_matrix = m_shadow_sbpv_matrix * m_roomMeshes[i].m_modelMat;
		glUniformMatrix4fv(SceneManager::Instance()->m_shadowMatHandle, 1, false, glm::value_ptr(shadow_matrix));

		glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(m_roomMeshes[i].m_modelMat));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_roomMeshes[i].material.diffuse_tex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowmap.depth_tex);

		glUniform3fv(SceneManager::Instance()->m_lightPositionHandle, 1, value_ptr(SceneManager::Instance()->light_position));
		glUniform3fv(SceneManager::Instance()->m_ambientAlbedoHandle, 1, value_ptr(m_roomMeshes[i].material.Ka));
		glUniform3fv(SceneManager::Instance()->m_diffuseAlbedoHandle, 1, value_ptr(m_roomMeshes[i].material.Kd));
		glUniform3fv(SceneManager::Instance()->m_specularAlbedoHandle, 1, value_ptr(m_roomMeshes[i].material.Ks));
		glUniform1f(SceneManager::Instance()->m_shininessHandle, m_roomMeshes[i].material.shininess);

		glDrawElements(GL_TRIANGLES, m_roomMeshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}

	glUniform1i(20, 1);
	for (int i = 0;i < m_triceMeshes.size();++i) {

		glBindVertexArray(m_triceMeshes[i].vao);

		shadow_matrix = m_shadow_sbpv_matrix * m_triceMeshes[i].m_modelMat;
		glUniformMatrix4fv(SceneManager::Instance()->m_shadowMatHandle, 1, false, glm::value_ptr(shadow_matrix));


		glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(m_triceMeshes[i].m_modelMat));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_triceMeshes[i].material.diffuse_tex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowmap.depth_tex);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_triceMeshes[i].material.normalmap_tex);

		glUniform3fv(SceneManager::Instance()->m_lightPositionHandle, 1, value_ptr(SceneManager::Instance()->light_position));
		glUniform3fv(SceneManager::Instance()->m_ambientAlbedoHandle, 1, value_ptr(m_triceMeshes[i].material.Ka));
		glUniform3fv(SceneManager::Instance()->m_diffuseAlbedoHandle, 1, value_ptr(m_triceMeshes[i].material.Kd));
		glUniform3fv(SceneManager::Instance()->m_specularAlbedoHandle, 1, value_ptr(m_triceMeshes[i].material.Ks));
		glUniform1f(SceneManager::Instance()->m_shininessHandle, m_triceMeshes[i].material.shininess);

		glDrawElements(GL_TRIANGLES, m_triceMeshes[i].drawCount, GL_UNSIGNED_INT, nullptr);
	}
}

namespace DEFERRED_MAP_TYPE {
	const GLint WORLD_COORD = 0;
	const GLint NORMAL = 1;
	const GLint AMBIENT = 2;
	const GLint DIFFUSE = 3;
	const GLint SPECULAR = 4;

}