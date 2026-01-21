#ifndef ENGINE_H
#define ENGINE_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Rasterizer.h"
#include "Transform.h"
#include "Shaders.h"

/**
 * @brief Main Engine class for Lumina3D
 * 
 * This class manages the window, input, and rendering loop.
 * It coordinates all subsystems including rasterization, transformations, and shading.
 */
class Engine {
public:
    // Window dimensions
    static const int WINDOW_WIDTH = 1600;
    static const int WINDOW_HEIGHT = 900;
    static const int VIEWPORT_WIDTH = 800;  // Right side viewport
    static const int VIEWPORT_HEIGHT = 900;
    
    // Constructor and Destructor
    Engine();
    ~Engine();
    
    // Main engine loop
    bool initialize();
    void run();
    void shutdown();
    
    // Input handling
    void processInput();
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    
    // Getters
    GLFWwindow* getWindow() const { return window; }
    Rasterizer* getRasterizer() const { return rasterizer; }
    Transform* getTransform() const { return transform; }
    
private:
    GLFWwindow* window;
    Rasterizer* rasterizer;
    Transform* transform;
    
    // OpenGL texture for displaying frame buffer
    GLuint frameTexture;
    
    // Rotation and scale for interactive demo
    float rotationX;
    float rotationY;
    float rotationZ;
    float scale;
    
    // Camera parameters
    glm::vec3 cameraPos;
    glm::vec3 cameraTarget;
    glm::vec3 cameraUp;
    
    // Light parameters
    glm::vec3 lightPos;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;
    
    // Rendering methods
    void update(float deltaTime);
    void render();
    void renderScene();
    void renderUI();
    void drawCube();
    void drawMoon();
    void drawMoonTriangle(const glm::vec4& v1, const glm::vec4& v2, const glm::vec4& v3,
                          const glm::vec3& n1, const glm::vec3& n2, const glm::vec3& n3,
                          const Light& light, const Material& material);
    float generateCraterDisplacement(float theta, float phi);
    
    // Helper methods
    void setupDefaultScene();
};

#endif // ENGINE_H
