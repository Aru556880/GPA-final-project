#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Rendering_Framework\src\SceneManager.h>
#include "Rendering_Framework\src\Common.h"

class IndoorSceneObject 
{
public:
	IndoorSceneObject();
	virtual ~IndoorSceneObject();

	void update();
	void init();

public:
	

public:
	

private:
	//GLuint m_normalMapHandle;
	//GLuint m_albedoMapHandle;

	vector<Shape> m_shapes;
	mat4 m_modelMat = mat4(1.0);
	GLuint m_pixelFunctionId = 0;
};

