#pragma once

#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw.h>
#include <imgui\imgui_impl_opengl3.h>
#include "MyCameraManager.h"

#include <string>

class MyImGuiPanel
{
public:
	MyImGuiPanel(INANOA::MyCameraManager* cameraManager);
	virtual ~MyImGuiPanel();

public:
	void update();
	void setAvgFPS(const double avgFPS);
	void setAvgFrameTime(const double avgFrameTime);
private:
	double m_avgFPS;
	double m_avgFrameTime;
	INANOA::MyCameraManager* m_cameraManager;
};

