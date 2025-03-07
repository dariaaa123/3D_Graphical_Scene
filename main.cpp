#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm.hpp> //core glm functionality
#include <gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>
#include "SkyBox.hpp"

// window
gps::Window myWindow;
int cameraAnimationOn=0;
// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

glm::mat4 rotateLight;
float angleLight = 0;


// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

GLint activatePointLight;
GLint activateFog;
// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
GLfloat angle;

// shaders
gps::Shader myBasicShader;

std::vector<glm::vec3> controlPoints = {
    glm::vec3(-33.6f, -29.6f, 1.0f),
    glm::vec3(33.6f, -29.6f, 1.0f),
    glm::vec3(33.6f, 29.6f, 1.0f),
    glm::vec3(-33.6f, 29.6f, 1.0f)
};

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

enum RenderMode { SOLID, WIREFRAME, POLYGON, SMOOTH };
RenderMode currentMode = SOLID;

// Adăugăm parametri pentru lumina punctuală
glm::vec3 lightPos2 = glm::vec3(3.0f, 15.0f, 2.0f); // Poziția luminii punctuale
glm::vec3 lightColor2 = glm::vec3(1.0f, 0.5f, 0.5f); // Reddish

// Uniform locations
GLint lightDir1Loc, lightColor1Loc;
GLint lightPos2Loc, lightColor2Loc;


glm::vec3 pointLightPos = glm::vec3(2.0f, 2.0f, 2.0f);
glm::vec3 pointLightColor = glm::vec3(1.0f, 0.5f, 0.5f);
float constant = 1.0f;
float linear = 0.09f;
float quadratic = 0.032f;

double lastX = 512.0, lastY = 384.0; // Assume the initial position is at the screen's center
bool firstMouse = true;

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    //TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        // First frame, initialize the lastX and lastY
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Calculate the offset
    double xOffset = xpos - lastX;
    double yOffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // Sensitivity for smooth movement
    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    // Update the camera's yaw and pitch
    myCamera.rotate(yOffset, xOffset); // Assuming rotate modifies the camera direction

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}


void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_1]) {
        angleLight -= 1.0f;

    }

    if (pressedKeys[GLFW_KEY_2]) {
        angleLight += 1.0f;

    }

    if (pressedKeys[GLFW_KEY_5]) {
        myBasicShader.useShaderProgram();
        glUniform1i(activateFog, 1);

    }

    if (pressedKeys[GLFW_KEY_6]) {
        myBasicShader.useShaderProgram();
        glUniform1i(activateFog, 0);

    }

    if (pressedKeys[GLFW_KEY_3]) {
        myBasicShader.useShaderProgram();
        glUniform1i( activatePointLight , 1);

    }
    if (pressedKeys[GLFW_KEY_4]) {
        myBasicShader.useShaderProgram();
        glUniform1i(activatePointLight, 0);

    }
    if (pressedKeys[GLFW_KEY_Y]) {
        //myCamera.toggleAnimation();
        cameraAnimationOn = 1;
    }
    if (pressedKeys[GLFW_KEY_T]) {

        //myCamera.toggleAnimation();
        cameraAnimationOn = 0;
    }

    if (pressedKeys[GLFW_KEY_Z]) {
        //myCamera.toggleAnimation();
        currentMode = SOLID;
    }

    if (pressedKeys[GLFW_KEY_X]) {
        //myCamera.toggleAnimation();
        currentMode = WIREFRAME;
    }

    if (pressedKeys[GLFW_KEY_C]) {
        //myCamera.toggleAnimation();
        currentMode = POLYGON;
    }


}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);

    // Set the cursor mode to disabled (useful for a free-look camera)
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initSkybox()
{
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");
    mySkyBox.Load(faces);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    //teapot.LoadModel("models/teapot/teapot20segUT.obj");
    teapot.LoadModel("models/house/zapada.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
}

void initUniforms() {
    myBasicShader.useShaderProgram();


    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(90.0f), // Wider FOV
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 50.0f); // Larger far plane

    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));


        // Lumina punctuală
        //lightPos2Loc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.position");
        //glUniform3fv(lightPos2Loc, 1, glm::value_ptr(lightPos2));

        ////lightColor2Loc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor2");
        ////glUniform3fv(lightColor2Loc, 1, glm::value_ptr(lightColor2));

        //GLint constantLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.constant");
        //glUniform1f(constantLoc, constant);

        //GLint linearLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.linear");
        //glUniform1f(linearLoc, linear);

        //GLint quadraticLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.quadratic");
        //glUniform1f(quadraticLoc, quadratic);
        //
        //activatePointLight = glGetUniformLocation(myBasicShader.shaderProgram, "activatePointLight");
        // Example C++ code to set PointLight uniforms
    glUniform3f(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.position"), 4.0f, 15.0f, 2.0f);
    glUniform3f(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.diffuse"), 0.5f, 0.5f, 0.5f);
    glUniform3f(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.specular"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.quadratic"), 0.032f);
    activatePointLight = glGetUniformLocation(myBasicShader.shaderProgram, "activatePointLight");
    activateFog = glGetUniformLocation(myBasicShader.shaderProgram, "activateFog");
    initSkybox();
}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* if (myCamera.isAnimationActive()) {
        myCamera.animateCamera(100);
    }*/

    //render the scene

    // render the teapot

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
    rotateLight = glm::rotate(glm::mat4(1.0f), glm::radians(angleLight), glm::vec3(20.0f, 1.0f, 0.0f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * rotateLight)) * lightDir));

 

    renderTeapot(myBasicShader);

    // glDepthMask(GL_FALSE);
    mySkyBox.Draw(skyboxShader, view, projection);
    //glDepthMask(GL_TRUE);

    switch (currentMode) {
    case SOLID:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    case WIREFRAME:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case POLYGON:
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    case SMOOTH:
        glShadeModel(GL_SMOOTH);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    }


   

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}
size_t currentPointIndex = 0;
bool cinematicActive;
float cinematicProgress;
void animateCamera(float deltaTime) {
    // Obținem punctele de start și sfârșit
    glm::vec3 start = controlPoints[currentPointIndex];
    glm::vec3 end = controlPoints[(currentPointIndex + 1) % controlPoints.size()];

    // Calculăm poziția interpolată între start și end
    glm::vec3 interpolatedPos = glm::mix(start, end, cinematicProgress);

    // Extragem coordonatele X, Y și Z
    float x = interpolatedPos.x;
    float y = interpolatedPos.y;
    float z = interpolatedPos.z;

    // Setăm poziția camerei folosind coordonatele extrase
    myCamera.setPosition(x, y, z);

    // Setăm direcția camerei către punctul final
    myCamera.setDirection(end);

    // Actualizăm progresul mișcării
    cinematicProgress += deltaTime * cameraSpeed;

    // Dacă progresul depășește 1.0, trecem la următorul segment
    if (cinematicProgress >= 1.0f) {
        cinematicProgress = 0.0f; // Resetăm progresul
        currentPointIndex++;

        // Dacă am ajuns la sfârșitul listei de puncte, oprim animația
        if (currentPointIndex >= controlPoints.size()) {
            cinematicActive = false; // Oprim animația
            currentPointIndex = 0;   // Resetăm indexul pentru o eventuală reluare
        }
    }
}


int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
       
        if (cameraAnimationOn == 1) {
            myCamera.setPosition(2.0f, 15.0f, 2.0f);
            float deltaTime = glfwGetTime();
            glfwSetTime(0); // Resetăm timpul pentru următorul frame
          animateCamera(deltaTime);
        }
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();          
    }

    cleanup();

    return EXIT_SUCCESS;
}