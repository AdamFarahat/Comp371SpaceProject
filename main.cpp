#include <iostream>
#include <vector>
#include <string>

//#define GLEW_STATIC 1 // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>  // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm.hpp>                  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <gtc/matrix_transform.hpp> // Include for glm::perspective and glm::lookAt
#include <gtc/type_ptr.hpp>         // Include for glm::value_ptr
#include "stb_image.h"
#include "skybox.h"
#include "camera.h"


// input handling functions
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
glm::vec3 startingCameraPos = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 startingCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 startingCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

Camera camera(startingCameraPos);

// set mouse cursor to center of window
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

//shaders here
const char* getVertexShaderSource() {
    return
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        //add another one in vec2 (2d, uv), for textures
        "layout (location = 2) in vec2 aText;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 vertexColor;\n"
        //here as well
        "out vec2 text;\n"
        "void main() {\n"
        "    vertexColor = aColor;\n"
        "    text = aText;\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}";
}


const char* getFragmentShaderSource() {
    return
        "#version 330 core\n"
        "in vec3 vertexColor;\n"
        //texture for frag here
        "in vec2 text;\n"
        "out vec4 FragColor;\n"
        //sampler here
        "uniform sampler2D sunTexture;\n"
        "void main() {\n"
        "    vec4 text = texture(sunTexture, text);\n"
        "    FragColor = text * vec4(vertexColor, 1.0);\n"
        "}";
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}

GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, getVertexShaderSource());
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, getFragmentShaderSource());

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void generateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, int sectorCount = 36, int stackCount = 18) {
    float radius = 1.0f;
    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectorCount; //using pi float included with glm 
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            //float u, for sector (horizontal), and v stack (vertical)
            
            float u = (float)j / sectorCount;
            float v = (float)i / stackCount;


            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(1.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
            //add in the u,v now
            vertices.push_back(u);
            vertices.push_back(v);

        }
    }

    for (int i = 0; i < stackCount; ++i) {
        for (int j = 0; j < sectorCount; ++j) {
            int first = i * (sectorCount + 1) + j;
            int second = first + sectorCount + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

//Load texture
GLuint loadTexture(const char* filename)
{
    //load texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
        return 0;
    }

    //return 0; //TODO: replace with texture loading code

      //create & bind textures
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    assert(textureId != 0);

    glBindTexture(GL_TEXTURE_2D, textureId);

    //set filter param
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    //upload texture to the PU
    GLenum format = 0;
    if (nrChannels == 1) {
        format = GL_RED;
    }
    else if (nrChannels == 3) {
        format = GL_RGB;
    }
    else if (nrChannels == 4) {
        format = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    //Free resources

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
}


int main()
{
    // Create a GLFW window and initialize GLEW
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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SPACE", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // window and mouse settings
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Create Skybox
    std::vector<std::string> faces{
        "skybox/right.png",
        "skybox/left.png",
        "skybox/top.png",
        "skybox/bottom.png",
        "skybox/front.png",
        "skybox/back.png"};
    unsigned int cubemapTexture = loadSkyBox(faces);
    glDepthMask(GL_FALSE);
    unsigned int skyboxVAO = createSkyboxVAO();
    // Load shaders
    int skyboxShader = compileAndLinkSkyboxShaders();
    glUseProgram(skyboxShader);
    // ... set view and projection matrix
    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    // Setup sphere
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    generateSphere(sphereVertices, sphereIndices);

    GLuint sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    //change attribute pointer to 8 floats, due to adding u,v for texture. + 1 more pointer for it too.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);//position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));//color
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));//UV
    glEnableVertexAttribArray(2);

    GLuint sunShader = createShaderProgram();

    // Set up view and projection matrices for camera
    glm::mat4 view = glm::mat4(glm::mat3(glm::lookAt(
        startingCameraPos,   // eye position
        startingCameraFront, // center position
        startingCameraUp     // up vector
        )));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            1920.0f / 1080.0f,
                                            0.1f, 100.0f);
    int viewLoc = glGetUniformLocation(skyboxShader, "view");
    int projLoc = glGetUniformLocation(skyboxShader, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    //spin the sun. (Praise the sun \[T]/ ) part 1 variable
    float sunRotation = 0.0f; //start at 0, first frame.

    //Texture for sun
    GLuint sunTextureID = loadTexture("Textures/sun.jpg");

    //depth and face cull
    glEnable(GL_DEPTH_TEST);   
    glDisable(GL_CULL_FACE);     


    // Main Loop
    while (!glfwWindowShouldClose(window))
    {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        GLuint projectionMatrixLocation = glGetUniformLocation(skyboxShader, "projection");
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projection[0][0]);

        view = camera.GetViewMatrix();
        GLuint viewMatrixLocation = glGetUniformLocation(skyboxShader, "view");
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &view[0][0]);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate matrices
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); // remove translation

        //Binding textures
        glUseProgram(sunShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTextureID);
        glUniform1i(glGetUniformLocation(sunShader, "sunTexture"), 0);

        // Draw Skybox
        glDepthFunc(GL_LEQUAL); // ensure skybox depth passes
        glDepthMask(GL_FALSE);
        glUseProgram(skyboxShader);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS); // restore default

        // Draw sun
        glUseProgram(sunShader);
        //glm::mat4 model = glm::mat4(1.0f);
        //actually spin the sun part 2 
        sunRotation += deltaTime * glm::radians(45.0f); // 45 degrees per second
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), sunRotation, glm::vec3(0.0f, 1.0f, 0.0f)); //spinning on y axis, so right to left, i hope.

        glUniformMatrix4fv(glGetUniformLocation(sunShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(sunShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(sunShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndices.size()), GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // Shutdown GLFW
    glfwTerminate();

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);


}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
