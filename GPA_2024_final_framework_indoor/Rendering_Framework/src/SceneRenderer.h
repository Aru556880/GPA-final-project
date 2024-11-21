#pragma once

#include <vector>
#include "Shader.h"
#include "SceneManager.h"
#include "indoorscene\IndoorSceneObject.h"
#include "Rendering_Framework\src\Common.h"

class SceneRenderer
{
public:
	SceneRenderer();
	virtual ~SceneRenderer();

private:
	glm::mat4 m_projMat;
	glm::mat4 m_viewMat;
	int m_frameWidth;
	int m_frameHeight;	

	IndoorSceneObject* m_indoorSO = nullptr;

public:
	void resize(const int w, const int h);
	bool initialize(const int w, const int h);

	void setProjection(const glm::mat4 &proj);
	void setView(const glm::mat4 &view);
	void setViewport(const int x, const int y, const int w, const int h);
	void appendIndoorSceneObject(IndoorSceneObject* indoorSO);

// pipeline
public:
	void startNewFrame();
	void renderPass();

private:
	void clear(const glm::vec4 &clearColor = glm::vec4(0.0, 0.0, 0.0, 1.0), const float depth = 1.0);
	bool setUpShader();

};

