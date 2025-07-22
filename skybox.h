#pragma once
#include <vector>
#include <string>


const char* getSkyboxVertexShaderSource();
const char* getSkyboxFragmentShaderSource();
GLuint createSkyboxVAO();
int compileAndLinkSkyboxShaders();
unsigned int loadSkyBox(std::vector<std::string> faces);
