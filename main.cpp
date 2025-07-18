#include <iostream>
#include <vector>
#include <string>


#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // Include for glm::perspective and glm::lookAt
#include "stb_image.h"
#include "skybox.h"
int main(){
//Create a GLFW window and initialize GLEW
    glfwInit();

#if defined(PLATFORM_OSX)	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	// On windows, we set OpenGL version to 2.1, to support more hardware
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "SPACE", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    //Create Skybox
    std::vector<std::string> faces
    {
        "skybox/right.png",
        "skybox/left.png",
        "skybox/top.png",
        "skybox/bottom.png",
        "skybox/front.png",
        "skybox/back.png"
    };
    unsigned int cubemapTexture = loadSkyBox(faces);
    glDepthMask(GL_FALSE);
    unsigned int skyboxVAO = createSkyboxVertexBufferObject();
    // Load shaders
    int skyboxShader = compileAndLinkSkyboxShaders();
    glUseProgram(skyboxShader);
    // ... set view and projection matrix
    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    // Set up view and projection matrices for camera
    glm::mat4 view = glm::mat4(glm::mat3(glm::lookAt(
        glm::vec3(0.0f, 0.0f, 0.0f), // eye position
        glm::vec3(0.0f, 0.0f, -1.0f), // center position
        glm::vec3(0.0f, 1.0f, 0.0f)   // up vector
    )));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                        1920.0f/1080.0f, 
                                        0.1f, 100.0f);
    int viewLoc = glGetUniformLocation(skyboxShader, "view");
    int projLoc = glGetUniformLocation(skyboxShader, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);


    //Main Loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glDepthMask(GL_FALSE);
        glUseProgram(skyboxShader);
        
        // Set view and projection here
        
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    // Shutdown GLFW
    glfwTerminate();
    
	return 0;
}