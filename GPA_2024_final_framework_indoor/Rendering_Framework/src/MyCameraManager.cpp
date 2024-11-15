#include "MyCameraManager.h"
#include <glm\gtc\matrix_transform.hpp>
#include "Rendering_Framework\src\Common.h"
#include <Rendering_Framework\src\SceneManager.h>
namespace INANOA {
MyCameraManager::MyCameraManager() : MIN_PLAYER_CAMERA_TERRAIN_OFFSET(5.0), MIN_AIRPLANE_TERRAIN_OFFSET(3.0)
{
}


MyCameraManager::~MyCameraManager()
{
}

void MyCameraManager::init(const int w, const int h){
	// initialize camera and camera control
	this->m_playerCameraForwardSpeed = 0.025;
	// initialize player camera
	this->m_playerMyCamera = new INANOA::MyCamera(glm::vec3(4.0, 1.0, -1.5), glm::vec3(3.0, 1.0, -1.5), glm::vec3(0.0, 1.0, 0.0), -1.0);
	this->resize(w, h);
}
void MyCameraManager::resize(const int w, const int h){
	// half for god view, half for player view
	const float PLAYER_PROJ_FAR = 700.0;
	this->setupViewports(w, h);

	this->m_playerProjMat = glm::perspective(glm::radians(45.0f), this->m_playerViewport[2] * 1.0f / h, 0.1f, PLAYER_PROJ_FAR);
}

void MyCameraManager::mousePress(const RenderWidgetMouseButton button, const int x, const int y) {
	if (button == RenderWidgetMouseButton::M_LEFT) {

	}
	else if (button == RenderWidgetMouseButton::M_RIGHT) {

	}
}
void MyCameraManager::mouseRelease(const RenderWidgetMouseButton button, const int x, const int y) {
	if (button == RenderWidgetMouseButton::M_LEFT) {

	}
	else if (button == RenderWidgetMouseButton::M_RIGHT) {

	}
}
void MyCameraManager::mouseMove(const int x, const int y) {

}
void MyCameraManager::mouseScroll(const double xOffset, const double yOffset) {

}
void MyCameraManager::keyPress(const RenderWidgetKeyCode key) {

	if (key == RenderWidgetKeyCode::KEY_W) {
		this->m_WPressedFlag = true;
	}
	else if (key == RenderWidgetKeyCode::KEY_S) {
		this->m_SPressedFlag = true;
	}
	else if (key == RenderWidgetKeyCode::KEY_A) {
		this->m_APressedFlag = true;
	}
	else if (key == RenderWidgetKeyCode::KEY_D) {
		this->m_DPressedFlag = true;
	}
	else if (key == RenderWidgetKeyCode::KEY_Z) {
		this->m_playerCameraHeightOffset = 0.1;
	}
	else if (key == RenderWidgetKeyCode::KEY_X) {
		this->m_playerCameraHeightOffset = -0.1;
	}	
	else if (key == RenderWidgetKeyCode::KEY_T) {
		SceneManager::Instance()->m_test_meshcount += 1;
		cout << "test" << SceneManager::Instance()->m_test_meshcount << endl;
	}
}
void MyCameraManager::keyRelease(const RenderWidgetKeyCode key) {
	if (key == RenderWidgetKeyCode::KEY_W) {
		this->m_WPressedFlag = false;
	}
	else if (key == RenderWidgetKeyCode::KEY_S) {
		this->m_SPressedFlag = false;
	}
	else if (key == RenderWidgetKeyCode::KEY_A) {
		this->m_APressedFlag = false;
	}
	else if (key == RenderWidgetKeyCode::KEY_D) {
		this->m_DPressedFlag = false;
	}

	if (key == RenderWidgetKeyCode::KEY_Z || key == RenderWidgetKeyCode::KEY_X) {
		this->m_playerCameraHeightOffset = 0.0;
	}
}

void MyCameraManager::updatePlayerCamera() {

	// update player camera
	if (this->m_WPressedFlag) {
		glm::vec3 before = this->m_playerMyCamera->lookCenter();
		this->m_playerMyCamera->forward(glm::vec3(0.0, 0.0, -1.0) * this->m_playerCameraForwardSpeed, true);
		glm::vec3 after = this->m_playerMyCamera->lookCenter();

	}
	else if (this->m_SPressedFlag) {
		glm::vec3 before = this->m_playerMyCamera->lookCenter();
		this->m_playerMyCamera->forward(glm::vec3(0.0, 0.0, 1.0) * this->m_playerCameraForwardSpeed, true);
		glm::vec3 after = this->m_playerMyCamera->lookCenter();
	}
	else if (this->m_APressedFlag) {
		this->m_playerMyCamera->rotateLookCenterAccordingToViewOrg(0.01);
	}
	else if (this->m_DPressedFlag) {
		this->m_playerMyCamera->rotateLookCenterAccordingToViewOrg(-0.01);
	}

	this->m_playerMyCamera->translateLookCenterAndViewOrg(glm::vec3(0.0, this->m_playerCameraHeightOffset, 0.0));
	this->m_playerMyCamera->update();	
}

void MyCameraManager::setupViewports(const int w, const int h) {
	this->m_playerViewport[2] = w;
	this->m_playerViewport[0] = w - this->m_playerViewport[2];
	this->m_playerViewport[1] = 0;
	this->m_playerViewport[3] = h;

}

// ===============================
glm::mat4 MyCameraManager::playerViewMatrix() const { return this->m_playerMyCamera->viewMatrix(); }
glm::mat4 MyCameraManager::playerProjectionMatrix() const { return this->m_playerProjMat; }
glm::vec3 MyCameraManager::playerViewOrig() const { return this->m_playerMyCamera->viewOrig(); }
glm::vec3 MyCameraManager::playerCameraLookCenter() const { return this->m_playerMyCamera->lookCenter(); }

glm::ivec4 MyCameraManager::playerViewport() const { return this->m_playerViewport; }

float MyCameraManager::playerCameraNear() const { return 0.1; }
float MyCameraManager::playerCameraFar() const { return 400.0; }

}

