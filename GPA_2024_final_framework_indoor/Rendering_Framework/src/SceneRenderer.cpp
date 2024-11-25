#include "SceneRenderer.h"


SceneRenderer::SceneRenderer()
{
}


SceneRenderer::~SceneRenderer()
{
}
void SceneRenderer::startNewFrame() {
	this->clear();
}
void SceneRenderer::renderPass(){
	this->m_indoorSO->set_mvMat(m_viewMat, m_projMat);

	if (this->m_indoorSO != nullptr) {
		//glUniform1i(SceneManager::Instance()->m_vs_vertexProcessIdHandle, SceneManager::Instance()->m_vs_commonProcess);
		this->m_indoorSO->update();
	}
}

// =======================================
void SceneRenderer::resize(const int w, const int h){
	this->m_frameWidth = w;
	this->m_frameHeight = h;

	if(m_indoorSO != nullptr)
		m_indoorSO->resize(w, h);
}
bool SceneRenderer::initialize(const int w, const int h){
	
	this->resize(w, h);
	const bool flag = this->setUpShader();
	
	if (!flag) {
		return false;
	}	
	
	glEnable(GL_DEPTH_TEST);

	return true;
}
void SceneRenderer::setProjection(const glm::mat4 &proj){
	this->m_projMat = proj;
}
void SceneRenderer::setView(const glm::mat4 &view){
	this->m_viewMat = view;
}
void SceneRenderer::setViewport(const int x, const int y, const int w, const int h) {
	glViewport(x, y, w, h);
}

void SceneRenderer::appendIndoorSceneObject(IndoorSceneObject* indoorSO) {
	this->m_indoorSO = indoorSO;
}
void SceneRenderer::clear(const glm::vec4 &clearColor, const float depth){
	static const float COLOR[] = { 0.0, 0.0, 0.0, 1.0 };
	static const float DEPTH[] = { 1.0 };

	glClearBufferfv(GL_COLOR, 0, COLOR);
	glClearBufferfv(GL_DEPTH, 0, DEPTH);
}
bool SceneRenderer::setUpShader(){

	// shader attributes binding

	SceneManager *manager = SceneManager::Instance();
	// vertex attribute 
	manager->m_vertexHandle = 0;
	manager->m_normalHandle = 1;
	manager->m_uvHandle = 2;

	// =================================
	// transform matrix uniform location
	manager->m_modelMatHandle = 0;
	manager->m_viewMatHandle = 1;
	manager->m_projMatHandle = 2;
	manager->m_shadowMatHandle = 3;

	manager->m_albedoMapHandle = 4; // uniform location
	manager->m_albedoMapTexIdx = 0; // texture binding point
	glUniform1i(manager->m_albedoMapHandle, manager->m_albedoMapTexIdx);
	
	manager->m_normalMapHandle = 6; // uniform location
	manager->m_normalMapTexIdx = 2; // texture binding point
	glUniform1i(manager->m_normalMapHandle, manager->m_normalMapTexIdx);

	manager->m_depthMapHandle = 5;
	
	manager->m_albedoTexUnit = GL_TEXTURE0;
	manager->m_normalTexUnit = GL_TEXTURE2;



	manager->m_lightPositionHandle = 11;
	manager->m_ambientAlbedoHandle = 13;
	manager->m_diffuseAlbedoHandle = 14;
	manager->m_specularAlbedoHandle = 15;
	manager->m_shininessHandle = 16;

	manager->m_shadowMatricesHandle = 17;
	manager->m_farPlaneHandle = 18;

	manager->m_useNormalMapHandle = 20;
	
	return true;
}
