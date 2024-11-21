#pragma once

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#include <string> 
#include <iostream>
#include <vector>

using namespace std;
using namespace glm;

using std::string;
using std::cerr;
using std::endl;
using std::vector;

typedef struct _texture_data
{
    _texture_data() : width(0), height(0), data(0) {}
    int width;
    int height;
    unsigned char* data;
} texture_data;

struct Material
{
    GLuint diffuse_tex;
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float shininess;

};

struct MyMesh
{
    GLuint vao;
    GLuint vbo_position;
    GLuint vbo_normal;
    GLuint vbo_texcoord;
    GLuint ibo;
    int drawCount;
    Material material;
};


texture_data loadImg(const char* path);
void LoadModel(vector<MyMesh>& shapes, string filePath, uint start_mesh, uint end_mesh);