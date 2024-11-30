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

	ImGui::SliderFloat3("Eye Position", &(m_cameraManager->playerViewOrig_ref()->x), -5.0, 5.0);
	ImGui::SliderFloat3("Look Center", &(m_cameraManager->playerCameraLookCenter_ref()->x), -5.0, 5.0);
	ImGui::Checkbox("Enable Normal Map", &SceneManager::Instance()->renderFeature.enableNormalMap);
	
	ImGui::PushID(static_cast<int>(0));
	if (ImGui::CollapsingHeader("Post Process")) {
		ImGui::Checkbox("Enable FXAA", &SceneManager::Instance()->renderFeature.postProcess.enableFXAA);
		ImGui::Checkbox("Enable Bloom Effect", &SceneManager::Instance()->renderFeature.postProcess.enableBloomEffect);
	}
	ImGui::PopID();

	ImGui::PushID(static_cast<int>(1));
	if (ImGui::CollapsingHeader("Blinn Phong Lighting")) {
		ImGui::SliderFloat3("Light Position", &(SceneManager::Instance()->renderFeature.blinnPhongLight.lightPos[0]), -5.0f, 5.0f);
		ImGui::Checkbox("Enable Light", &SceneManager::Instance()->renderFeature.blinnPhongLight.enableLight);
		ImGui::Checkbox("Enable Shadow", &SceneManager::Instance()->renderFeature.blinnPhongLight.enableShadow);
		ImGui::Checkbox("Enable Volumetric Light", &SceneManager::Instance()->renderFeature.postProcess.enableVolumetricLight);
	}
	ImGui::PopID();

	ImGui::PushID(static_cast<int>(2));
	if (ImGui::CollapsingHeader("Point Light")) {
		ImGui::SliderFloat3("Light Position", &(SceneManager::Instance()->renderFeature.pointLight.lightPos[0]), -5.0f, 5.0f);
		ImGui::Checkbox("Enable Light", &SceneManager::Instance()->renderFeature.pointLight.enableLight);
		ImGui::Checkbox("Enable Shadow", &SceneManager::Instance()->renderFeature.pointLight.enableShadow);
	}
	ImGui::PopID();

	ImGui::PushID(static_cast<int>(3));
	if (ImGui::CollapsingHeader("Area Light")) {
		ImGui::SliderFloat2("Rotation", &(SceneManager::Instance()->renderFeature.areaLight.lightRotate[0]), -5.0f, 5.0f);
		ImGui::SliderFloat3("Light Position", &(SceneManager::Instance()->renderFeature.areaLight.lightPos[0]), -5.0f, 5.0f);
		ImGui::Checkbox("Enable Light", &SceneManager::Instance()->renderFeature.areaLight.enableLight);
	}
	ImGui::PopID();

	ImGui::PushID(static_cast<int>(4));
	if (ImGui::CollapsingHeader("Deferred Shading")) {
		ImGui::Checkbox("Enable Deferred Map", &SceneManager::Instance()->renderFeature.deferredShading.enableDeferredMap);
		if (ImGui::BeginCombo("Select Map", SceneManager::Instance()->renderFeature.deferredShading.items[SceneManager::Instance()->renderFeature.deferredShading.current_item])) {
			for (int i = 0; i < 5; i++) {
				bool is_selected = (SceneManager::Instance()->renderFeature.deferredShading.current_item == i);
				if (ImGui::Selectable(SceneManager::Instance()->renderFeature.deferredShading.items[i], is_selected)) {
					SceneManager::Instance()->renderFeature.deferredShading.current_item = i;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}
	ImGui::PopID();
}

void MyImGuiPanel::setAvgFPS(const double avgFPS){
	this->m_avgFPS = avgFPS;
}
void MyImGuiPanel::setAvgFrameTime(const double avgFrameTime){
	this->m_avgFrameTime = avgFrameTime;
}