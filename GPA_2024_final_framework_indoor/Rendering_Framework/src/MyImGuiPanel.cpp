#include "MyImGuiPanel.h"
#include <Rendering_Framework\src\SceneManager.h>


MyImGuiPanel::MyImGuiPanel(INANOA::MyCameraManager* cameraManager)
{
	this->m_avgFPS = 0.0;
	this->m_avgFrameTime = 0.0;
	m_cameraManager = cameraManager;
}


MyImGuiPanel::~MyImGuiPanel()
{
}

void MyImGuiPanel::update() {
	// performance information
	const std::string FPS_STR = "FPS: " + std::to_string(this->m_avgFPS);
	ImGui::TextColored(ImVec4(0, 220, 0, 255), FPS_STR.c_str());
	const std::string FT_STR = "Frame: " + std::to_string(this->m_avgFrameTime);
	ImGui::TextColored(ImVec4(0, 220, 0, 255), FT_STR.c_str());

	ImGui::InputFloat3("Eye Position", &(m_cameraManager->playerViewOrig_ref()->x));
	ImGui::InputFloat3("Look Center", &(m_cameraManager->playerCameraLookCenter_ref()->x));

}

void MyImGuiPanel::setAvgFPS(const double avgFPS){
	this->m_avgFPS = avgFPS;
}
void MyImGuiPanel::setAvgFrameTime(const double avgFrameTime){
	this->m_avgFrameTime = avgFrameTime;
}