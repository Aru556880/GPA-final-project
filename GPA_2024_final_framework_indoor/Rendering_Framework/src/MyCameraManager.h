#pragma once

#include "camera\MyOrbitControl.h"

enum class RenderWidgetKeyCode {
	KEY_W, KEY_A, KEY_S, KEY_D, KEY_T, KEY_Z, KEY_X
};
enum class RenderWidgetMouseButton {
	M_LEFT, M_RIGHT, M_MIDDLE
};

namespace INANOA {

class MyCameraManager
{
public:
	MyCameraManager();
	virtual ~MyCameraManager();

public:
	void init(const int w, const int h);
	void resize(const int w, const int h);

	void mousePress(const RenderWidgetMouseButton button, const int x, const int y) ;
	void mouseRelease(const RenderWidgetMouseButton button, const int x, const int y) ;
	void mouseMove(const int x, const int y) ;
	void mouseScroll(const double xOffset, const double yOffset) ;
	void keyPress(const RenderWidgetKeyCode key) ;
	void keyRelease(const RenderWidgetKeyCode key) ;

public:
	void updatePlayerCamera();

private:
	
	void setupViewports(const int w, const int h);

public:
	glm::mat4 playerViewMatrix() const;
	glm::mat4 playerProjectionMatrix() const;
	glm::vec3 playerViewOrig() const;
	glm::vec3 playerCameraLookCenter() const;


	glm::ivec4 playerViewport() const;

	float playerCameraNear() const;
	float playerCameraFar() const;



	

private:
	glm::mat4 m_airplaneModelMat;

	MyCamera* m_playerMyCamera = nullptr;

	glm::mat4 m_playerProjMat;

	float m_playerCameraHeightOffset = 0.0;
	float m_playerCameraForwardSpeed = 0.5;

	const float MIN_PLAYER_CAMERA_TERRAIN_OFFSET;
	const float MIN_AIRPLANE_TERRAIN_OFFSET;

	glm::ivec4 m_playerViewport;

private:
	bool m_WPressedFlag = false;
	bool m_APressedFlag = false;
	bool m_SPressedFlag = false;
	bool m_DPressedFlag = false;

};


}



