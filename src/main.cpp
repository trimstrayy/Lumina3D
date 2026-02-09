#include "Engine.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// Global pointer for callbacks
Engine* g_engine = nullptr;

/**
 * @brief Constructor
 */
Engine::Engine() 
    : window(nullptr), 
      rasterizer(nullptr), 
      transform(nullptr),
      rotationX(0.0f), 
      rotationY(0.0f), 
      rotationZ(0.0f), 
      scale(1.0f) {
    g_engine = this;
}

/**
 * @brief Destructor
 */
Engine::~Engine() {
    shutdown();
}

/**
 * @brief Initializes GLFW, creates window, and sets up subsystems
 */
bool Engine::initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Set OpenGL version (2.1 for maximum compatibility with immediate mode)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // Don't specify core profile to allow legacy functions
    
    // Create window
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 
                             "Lumina3D Engine - COMP 342", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    
    // Create texture for frame buffer display
    glGenTextures(1, &frameTexture);
    glBindTexture(GL_TEXTURE_2D, frameTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    // Initialize subsystems
    rasterizer = new Rasterizer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    transform = new Transform();
    
    // Setup default scene
    setupDefaultScene();
    
    // Initialize OpenGL state
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Print OpenGL info for debugging
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    std::cout << "Lumina3D Engine Initialized" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Arrow Keys: Rotate object" << std::endl;
    std::cout << "  +/- : Scale object" << std::endl;
    std::cout << "  R : Reset transformations" << std::endl;
    std::cout << "  ESC : Exit" << std::endl;
    
    return true;
}

/**
 * @brief Main engine loop
 */
void Engine::run() {
    float lastTime = static_cast<float>(glfwGetTime());
    
    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // Process input
        processInput();
        
        // Update
        update(deltaTime);
        
        // Render
        render();
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

/**
 * @brief Cleans up resources
 */
void Engine::shutdown() {
    if (rasterizer) {
        delete rasterizer;
        rasterizer = nullptr;
    }
    
    if (transform) {
        delete transform;
        transform = nullptr;
    }
    
    if (frameTexture) {
        glDeleteTextures(1, &frameTexture);
    }
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    
    glfwTerminate();
}

/**
 * @brief Sets up camera, lighting, and initial transformations
 */
void Engine::setupDefaultScene() {
    // Camera setup
    cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    transform->setLookAt(cameraPos, cameraTarget, cameraUp);
    
    // Projection setup (perspective)
    float aspect = static_cast<float>(VIEWPORT_WIDTH) / static_cast<float>(VIEWPORT_HEIGHT);
    transform->setPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    
    // Light setup
    lightPos = glm::vec3(5.0f, 3.0f, 5.0f);
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    ambientColor = glm::vec3(0.3f, 0.3f, 0.3f);
}

/**
 * @brief Update loop - updates transformations
 */
void Engine::update(float deltaTime) {
    // Create model matrix with current transformations
    glm::mat4 model = glm::mat4(1.0f);
    model = transform->createScaleMatrix(scale, scale, scale) * model;
    model = transform->createRotationMatrix(rotationX, rotationY, rotationZ) * model;
    
    transform->setModelMatrix(model);
}

/**
 * @brief Main render loop
 */
void Engine::render() {
    // Clear to black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup for 2D rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Render software rasterized scene
    rasterizer->clearBuffers(Color(0, 0, 0, 255));
    renderScene();
    
    // Upload framebuffer to texture
    glBindTexture(GL_TEXTURE_2D, frameTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 
                 0, GL_RGB, GL_UNSIGNED_BYTE, rasterizer->getFrameBuffer());
    
    // Draw textured quad on right side
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    
    float x = WINDOW_WIDTH - VIEWPORT_WIDTH;
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x, 0);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(WINDOW_WIDTH, 0);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(WINDOW_WIDTH, VIEWPORT_HEIGHT);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x, VIEWPORT_HEIGHT);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    
    // Render UI on left side
    renderUI();
}

/**
 * @brief Renders the 3D scene (moon sphere with transformations)
 */
void Engine::renderScene() {
    // Draw a moon sphere with craters
    drawMoon();
}

/**
 * @brief Draws a small sphere at the light position to visualize the light source
 */
void Engine::drawLightSource() {
    const int latSegments = 10;
    const int lonSegments = 10;
    const float radius = 0.15f;
    
    Light light;
    light.position = lightPos;
    light.color = lightColor;
    light.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
    
    Material material;
    material.diffuse = glm::vec3(1.0f, 1.0f, 0.0f);  // Bright yellow
    material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    material.shininess = 32.0f;
    
    // Generate simple sphere at light position
    for (int lat = 0; lat < latSegments; ++lat) {
        for (int lon = 0; lon < lonSegments; ++lon) {
            float theta1 = lat * 3.14159f / latSegments;
            float theta2 = (lat + 1) * 3.14159f / latSegments;
            float phi1 = lon * 2.0f * 3.14159f / lonSegments;
            float phi2 = (lon + 1) * 2.0f * 3.14159f / lonSegments;
            
            auto generateVertex = [&](float theta, float phi) -> std::pair<glm::vec4, glm::vec3> {
                float x = lightPos.x + radius * std::sin(theta) * std::cos(phi);
                float y = lightPos.y + radius * std::cos(theta);
                float z = lightPos.z + radius * std::sin(theta) * std::sin(phi);
                
                glm::vec3 normal = glm::normalize(glm::vec3(
                    radius * std::sin(theta) * std::cos(phi),
                    radius * std::cos(theta),
                    radius * std::sin(theta) * std::sin(phi)
                ));
                
                return {glm::vec4(x, y, z, 1.0f), normal};
            };
            
            auto [v1, n1] = generateVertex(theta1, phi1);
            auto [v2, n2] = generateVertex(theta1, phi2);
            auto [v3, n3] = generateVertex(theta2, phi2);
            auto [v4, n4] = generateVertex(theta2, phi1);
            
            // Draw bright yellow triangles for light indicator
            drawLightTriangle(v1, v2, v3);
            drawLightTriangle(v1, v3, v4);
        }
    }
}

/**
 * @brief Draws a triangle for the light source indicator (self-illuminated)
 */
void Engine::drawLightTriangle(const glm::vec4& v1, const glm::vec4& v2, const glm::vec4& v3) {
    // Transform vertices
    glm::vec4 v1Clip = transform->transformVertex(v1);
    glm::vec4 v2Clip = transform->transformVertex(v2);
    glm::vec4 v3Clip = transform->transformVertex(v3);
    
    // Perspective division
    glm::vec4 v1NDC = v1Clip / v1Clip.w;
    glm::vec4 v2NDC = v2Clip / v2Clip.w;
    glm::vec4 v3NDC = v3Clip / v3Clip.w;
    
    // Viewport transformation
    glm::vec2 v1Screen = transform->viewportTransform(v1NDC, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    glm::vec2 v2Screen = transform->viewportTransform(v2NDC, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    glm::vec2 v3Screen = transform->viewportTransform(v3NDC, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    // Create Vertex structures
    Vertex vert1, vert2, vert3;
    
    vert1.position = glm::vec4(v1Screen.x, v1Screen.y, v1NDC.z, 1.0f);
    vert2.position = glm::vec4(v2Screen.x, v2Screen.y, v2NDC.z, 1.0f);
    vert3.position = glm::vec4(v3Screen.x, v3Screen.y, v3NDC.z, 1.0f);
    
    // Bright yellow color (self-illuminated, no shading needed)
    vert1.color = Color(255, 255, 100);
    vert2.color = Color(255, 255, 100);
    vert3.color = Color(255, 255, 100);
    
    // Rasterize the triangle
    rasterizer->drawTriangle(vert1, vert2, vert3, true);
}

/**
 * @brief Generates crater displacement for moon surface
 */
float Engine::generateCraterDisplacement(float theta, float phi) {
    // Create multiple craters at different positions
    struct Crater {
        float theta, phi, radius, depth;
    };
    
    std::vector<Crater> craters = {
        {0.5f, 0.8f, 0.3f, 0.15f},     // Large crater
        {2.0f, 1.5f, 0.25f, 0.12f},    // Medium crater
        {-1.0f, 0.5f, 0.2f, 0.10f},    // Small crater
        {1.5f, -0.7f, 0.35f, 0.18f},   // Large crater
        {-0.8f, -1.2f, 0.15f, 0.08f},  // Tiny crater
        {0.2f, -0.3f, 0.22f, 0.11f},   // Medium crater
        {-1.8f, 1.8f, 0.28f, 0.14f},   // Medium-large crater
        {2.5f, 0.2f, 0.18f, 0.09f}     // Small crater
    };
    
    float displacement = 0.0f;
    
    for (const auto& crater : craters) {
        float dTheta = theta - crater.theta;
        float dPhi = phi - crater.phi;
        float dist = std::sqrt(dTheta * dTheta + dPhi * dPhi);
        
        if (dist < crater.radius) {
            // Smooth crater profile using cosine function
            float normalized = dist / crater.radius;
            float craterDepth = crater.depth * (std::cos(normalized * 3.14159f) + 1.0f) * 0.5f;
            displacement -= craterDepth;
        }
    }
    
    // Add small surface roughness
    float roughness = 0.02f * std::sin(theta * 10.0f) * std::cos(phi * 10.0f);
    displacement += roughness;
    
    return displacement;
}

/**
 * @brief Draws a moon sphere with craters using manual triangle rasterization
 */
void Engine::drawMoon() {
    const int latSegments = 400;  // Latitude divisions (maximum resolution)
    const int lonSegments = 400;  // Longitude divisions (maximum resolution)
    const float radius = 2.0f;
    
    // Setup lighting
    Light light;
    light.position = lightPos;
    light.color = lightColor;
    light.ambient = ambientColor;
    
    Material material;
    material.diffuse = glm::vec3(0.8f, 0.8f, 0.75f);  // Moon gray color
    material.specular = glm::vec3(0.2f, 0.2f, 0.2f);  // Low specularity
    material.shininess = 8.0f;
    
    // Generate sphere with craters
    for (int lat = 0; lat < latSegments; ++lat) {
        for (int lon = 0; lon < lonSegments; ++lon) {
            // Calculate angles
            float theta1 = lat * 3.14159f / latSegments;
            float theta2 = (lat + 1) * 3.14159f / latSegments;
            float phi1 = lon * 2.0f * 3.14159f / lonSegments;
            float phi2 = (lon + 1) * 2.0f * 3.14159f / lonSegments;
            
            // Generate 4 vertices of the quad (will be split into 2 triangles)
            auto generateVertex = [&](float theta, float phi) -> std::pair<glm::vec4, glm::vec3> {
                float craterDisp = generateCraterDisplacement(theta, phi);
                float r = radius + craterDisp;
                
                float x = r * std::sin(theta) * std::cos(phi);
                float y = r * std::cos(theta);
                float z = r * std::sin(theta) * std::sin(phi);
                
                // Normal is direction from center for sphere
                glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
                
                return {glm::vec4(x, y, z, 1.0f), normal};
            };
            
            auto [v1, n1] = generateVertex(theta1, phi1);
            auto [v2, n2] = generateVertex(theta1, phi2);
            auto [v3, n3] = generateVertex(theta2, phi2);
            auto [v4, n4] = generateVertex(theta2, phi1);
            
            // Draw two triangles for each quad
            drawMoonTriangle(v1, v2, v3, n1, n2, n3, light, material);
            drawMoonTriangle(v1, v3, v4, n1, n3, n4, light, material);
        }
    }
}

/**
 * @brief Draws a single moon triangle with lighting
 */
void Engine::drawMoonTriangle(const glm::vec4& v1, const glm::vec4& v2, const glm::vec4& v3,
                               const glm::vec3& n1, const glm::vec3& n2, const glm::vec3& n3,
                               const Light& light, const Material& material) {
    // Transform vertices
    glm::vec4 v1Clip = transform->transformVertex(v1);
    glm::vec4 v2Clip = transform->transformVertex(v2);
    glm::vec4 v3Clip = transform->transformVertex(v3);
    
    // Perspective division
    glm::vec4 v1NDC = v1Clip / v1Clip.w;
    glm::vec4 v2NDC = v2Clip / v2Clip.w;
    glm::vec4 v3NDC = v3Clip / v3Clip.w;
    
    // Viewport transformation
    glm::vec2 v1Screen = transform->viewportTransform(v1NDC, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    glm::vec2 v2Screen = transform->viewportTransform(v2NDC, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    glm::vec2 v3Screen = transform->viewportTransform(v3NDC, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    // Create Vertex structures
    Vertex vert1, vert2, vert3;
    
    vert1.position = glm::vec4(v1Screen.x, v1Screen.y, v1NDC.z, 1.0f);
    vert2.position = glm::vec4(v2Screen.x, v2Screen.y, v2NDC.z, 1.0f);
    vert3.position = glm::vec4(v3Screen.x, v3Screen.y, v3NDC.z, 1.0f);
    
    // Calculate world positions
    glm::mat4 model = transform->getModelMatrix();
    vert1.worldPos = glm::vec3(model * v1);
    vert2.worldPos = glm::vec3(model * v2);
    vert3.worldPos = glm::vec3(model * v3);
    
    // Transform normals
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    vert1.normal = glm::normalize(normalMatrix * n1);
    vert2.normal = glm::normalize(normalMatrix * n2);
    vert3.normal = glm::normalize(normalMatrix * n3);
    
    // Calculate Gouraud shading colors
    vert1.color = Shaders::computeGouraudShading(vert1.worldPos, vert1.normal, 
                                                 cameraPos, light, material);
    vert2.color = Shaders::computeGouraudShading(vert2.worldPos, vert2.normal, 
                                                 cameraPos, light, material);
    vert3.color = Shaders::computeGouraudShading(vert3.worldPos, vert3.normal, 
                                                 cameraPos, light, material);
    
    // Rasterize the triangle
    rasterizer->drawTriangle(vert1, vert2, vert3, true);
}

/**
 * @brief Draws a cube using manual triangle rasterization (kept for reference)
 * 
 * A cube has 8 vertices and 12 triangles (2 per face, 6 faces)
 */
void Engine::drawCube() {
    // Define cube vertices in model space
    std::vector<glm::vec4> cubeVertices = {
        // Front face
        {-1.0f, -1.0f,  1.0f, 1.0f},  // 0
        { 1.0f, -1.0f,  1.0f, 1.0f},  // 1
        { 1.0f,  1.0f,  1.0f, 1.0f},  // 2
        {-1.0f,  1.0f,  1.0f, 1.0f},  // 3
        // Back face
        {-1.0f, -1.0f, -1.0f, 1.0f},  // 4
        { 1.0f, -1.0f, -1.0f, 1.0f},  // 5
        { 1.0f,  1.0f, -1.0f, 1.0f},  // 6
        {-1.0f,  1.0f, -1.0f, 1.0f}   // 7
    };
    
    // Define cube faces (triangle indices and colors)
    struct Face {
        int v1, v2, v3;
        Color color;
        glm::vec3 normal;
    };
    
    std::vector<Face> faces = {
        // Front face (red)
        {0, 1, 2, Color(200, 50, 50), glm::vec3(0, 0, 1)},
        {0, 2, 3, Color(200, 50, 50), glm::vec3(0, 0, 1)},
        // Right face (green)
        {1, 5, 6, Color(50, 200, 50), glm::vec3(1, 0, 0)},
        {1, 6, 2, Color(50, 200, 50), glm::vec3(1, 0, 0)},
        // Back face (blue)
        {5, 4, 7, Color(50, 50, 200), glm::vec3(0, 0, -1)},
        {5, 7, 6, Color(50, 50, 200), glm::vec3(0, 0, -1)},
        // Left face (yellow)
        {4, 0, 3, Color(200, 200, 50), glm::vec3(-1, 0, 0)},
        {4, 3, 7, Color(200, 200, 50), glm::vec3(-1, 0, 0)},
        // Top face (magenta)
        {3, 2, 6, Color(200, 50, 200), glm::vec3(0, 1, 0)},
        {3, 6, 7, Color(200, 50, 200), glm::vec3(0, 1, 0)},
        // Bottom face (cyan)
        {4, 5, 1, Color(50, 200, 200), glm::vec3(0, -1, 0)},
        {4, 1, 0, Color(50, 200, 200), glm::vec3(0, -1, 0)}
    };
    
    // Setup lighting for Gouraud shading
    Light light;
    light.position = lightPos;
    light.color = lightColor;
    light.ambient = ambientColor;
    
    Material material;
    
    // Draw each face
    for (const auto& face : faces) {
        // Transform vertices
        glm::vec4 v1Clip = transform->transformVertex(cubeVertices[face.v1]);
        glm::vec4 v2Clip = transform->transformVertex(cubeVertices[face.v2]);
        glm::vec4 v3Clip = transform->transformVertex(cubeVertices[face.v3]);
        
        // Perspective division (clip space -> NDC)
        glm::vec4 v1NDC = v1Clip / v1Clip.w;
        glm::vec4 v2NDC = v2Clip / v2Clip.w;
        glm::vec4 v3NDC = v3Clip / v3Clip.w;
        
        // Viewport transformation (NDC -> screen space)
        glm::vec2 v1Screen = transform->viewportTransform(v1NDC, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
        glm::vec2 v2Screen = transform->viewportTransform(v2NDC, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
        glm::vec2 v3Screen = transform->viewportTransform(v3NDC, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
        
        // Create Vertex structures for rasterization
        Vertex vert1, vert2, vert3;
        
        vert1.position = glm::vec4(v1Screen.x, v1Screen.y, v1NDC.z, 1.0f);
        vert2.position = glm::vec4(v2Screen.x, v2Screen.y, v2NDC.z, 1.0f);
        vert3.position = glm::vec4(v3Screen.x, v3Screen.y, v3NDC.z, 1.0f);
        
        // Calculate world positions for lighting
        glm::mat4 model = transform->getModelMatrix();
        vert1.worldPos = glm::vec3(model * cubeVertices[face.v1]);
        vert2.worldPos = glm::vec3(model * cubeVertices[face.v2]);
        vert3.worldPos = glm::vec3(model * cubeVertices[face.v3]);
        
        // Set normals and calculate Gouraud shading colors
        vert1.normal = face.normal;
        vert2.normal = face.normal;
        vert3.normal = face.normal;
        
        vert1.color = Shaders::computeGouraudShading(vert1.worldPos, vert1.normal, 
                                                     cameraPos, light, material);
        vert2.color = Shaders::computeGouraudShading(vert2.worldPos, vert2.normal, 
                                                     cameraPos, light, material);
        vert3.color = Shaders::computeGouraudShading(vert3.worldPos, vert3.normal, 
                                                     cameraPos, light, material);
        
        // Apply face color tint
        vert1.color.r = static_cast<uint8_t>(vert1.color.r * face.color.r / 255.0f);
        vert1.color.g = static_cast<uint8_t>(vert1.color.g * face.color.g / 255.0f);
        vert1.color.b = static_cast<uint8_t>(vert1.color.b * face.color.b / 255.0f);
        
        vert2.color.r = static_cast<uint8_t>(vert2.color.r * face.color.r / 255.0f);
        vert2.color.g = static_cast<uint8_t>(vert2.color.g * face.color.g / 255.0f);
        vert2.color.b = static_cast<uint8_t>(vert2.color.b * face.color.b / 255.0f);
        
        vert3.color.r = static_cast<uint8_t>(vert3.color.r * face.color.r / 255.0f);
        vert3.color.g = static_cast<uint8_t>(vert3.color.g * face.color.g / 255.0f);
        vert3.color.b = static_cast<uint8_t>(vert3.color.b * face.color.b / 255.0f);
        
        // Rasterize the triangle with Gouraud shading
        rasterizer->drawTriangle(vert1, vert2, vert3, true);
        
        // Optional: Draw wireframe
        // rasterizer->drawWireframeTriangle(vert1, vert2, vert3, Color(255, 255, 255));
    }
}

/**
 * @brief Helper function to render text at a specific position using pixel drawing
 * Simple monospace-like text rendering with rectangles
 */
void Engine::renderText(float x, float y, const std::string& text, float r, float g, float b) {
    // Simplified text rendering - just draw the position marker
    glColor3f(r, g, b);
    glPointSize(1.0f);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

/**
 * @brief Helper function to render a rectangle
 */
void Engine::drawRectangle(float x, float y, float width, float height, 
                          float r, float g, float b, bool filled) {
    glColor3f(r, g, b);
    
    if (filled) {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();
    }
}

/**
 * @brief Helper function to draw horizontal line
 */
void Engine::drawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float width) {
    glColor3f(r, g, b);
    glLineWidth(width);
    
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    
    glLineWidth(1.0f);
}

/**
 * @brief Renders UI overlay with controls and transformation info
 * 
 * This displays on the left side of the screen:
 * - Center cross pattern with 4 directional arrows
 * - Labels for R (reset) and ESC (exit) controls
 */
void Engine::renderUI() {
    // Setup for UI rendering
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    
    glColor3f(1.0f, 1.0f, 1.0f);  // White
    
    // Center cross pattern with arrows
    int centerX = 120;
    int centerY = 450;
    int arrowSpacing = 80;
    
    // Draw cross/diamond shape
    // Vertical line
    glBegin(GL_LINES);
    glVertex2f(centerX, centerY - arrowSpacing);
    glVertex2f(centerX, centerY + arrowSpacing);
    glEnd();
    
    // Horizontal line
    glBegin(GL_LINES);
    glVertex2f(centerX - arrowSpacing, centerY);
    glVertex2f(centerX + arrowSpacing, centerY);
    glEnd();
    
    // UP arrow (top)
    glBegin(GL_LINES);
    glVertex2f(centerX, centerY - arrowSpacing);
    glVertex2f(centerX - 8, centerY - arrowSpacing + 15);
    glVertex2f(centerX, centerY - arrowSpacing);
    glVertex2f(centerX + 8, centerY - arrowSpacing + 15);
    glEnd();
    
    // DOWN arrow (bottom)
    glBegin(GL_LINES);
    glVertex2f(centerX, centerY + arrowSpacing);
    glVertex2f(centerX - 8, centerY + arrowSpacing - 15);
    glVertex2f(centerX, centerY + arrowSpacing);
    glVertex2f(centerX + 8, centerY + arrowSpacing - 15);
    glEnd();
    
    // LEFT arrow (left)
    glBegin(GL_LINES);
    glVertex2f(centerX - arrowSpacing, centerY);
    glVertex2f(centerX - arrowSpacing + 15, centerY - 8);
    glVertex2f(centerX - arrowSpacing, centerY);
    glVertex2f(centerX - arrowSpacing + 15, centerY + 8);
    glEnd();
    
    // RIGHT arrow (right)
    glBegin(GL_LINES);
    glVertex2f(centerX + arrowSpacing, centerY);
    glVertex2f(centerX + arrowSpacing - 15, centerY - 8);
    glVertex2f(centerX + arrowSpacing, centerY);
    glVertex2f(centerX + arrowSpacing - 15, centerY + 8);
    glEnd();
    
    // === R Control Label (bottom left of cross) ===
    // Draw large "R" character
    int rX = 30;
    int rY = 540;
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // Left vertical line
    glVertex2f(rX, rY);
    glVertex2f(rX, rY + 40);
    // Top horizontal
    glVertex2f(rX, rY);
    glVertex2f(rX + 25, rY);
    // Top right curve (simplified)
    glVertex2f(rX + 25, rY);
    glVertex2f(rX + 25, rY + 20);
    // Middle horizontal
    glVertex2f(rX, rY + 20);
    glVertex2f(rX + 25, rY + 20);
    // Diagonal leg
    glVertex2f(rX + 25, rY + 20);
    glVertex2f(rX + 35, rY + 40);
    glEnd();
    glLineWidth(1.0f);
    
    // === ESC Control Label (bottom right of cross) ===
    // Draw large "E" character (for ESC)
    int escX = 160;
    int escY = 540;
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // Left vertical line
    glVertex2f(escX, escY);
    glVertex2f(escX, escY + 40);
    // Top horizontal
    glVertex2f(escX, escY);
    glVertex2f(escX + 25, escY);
    // Middle horizontal
    glVertex2f(escX, escY + 20);
    glVertex2f(escX + 25, escY + 20);
    // Bottom horizontal
    glVertex2f(escX, escY + 40);
    glVertex2f(escX + 25, escY + 40);
    glEnd();
    glLineWidth(1.0f);
    
    // === PLUS Control Label (below R) ===
    // Draw large "+" character
    int plusX = 30;
    int plusY = 620;
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // Vertical line
    glVertex2f(plusX + 17, plusY);
    glVertex2f(plusX + 17, plusY + 35);
    // Horizontal line
    glVertex2f(plusX, plusY + 17);
    glVertex2f(plusX + 35, plusY + 17);
    glEnd();
    glLineWidth(1.0f);
    
    // === MINUS Control Label (below E) ===
    // Draw large "-" character
    int minusX = 160;
    int minusY = 620;
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // Horizontal line
    glVertex2f(minusX, minusY + 17);
    glVertex2f(minusX + 35, minusY + 17);
    glEnd();
    glLineWidth(1.0f);
}

/**
 * @brief Processes keyboard input
 */
void Engine::processInput() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

/**
 * @brief GLFW keyboard callback
 */
void Engine::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        // Rotation controls
        if (key == GLFW_KEY_UP) {
            g_engine->rotationX += 0.1f;
        }
        if (key == GLFW_KEY_DOWN) {
            g_engine->rotationX -= 0.1f;
        }
        if (key == GLFW_KEY_LEFT) {
            g_engine->rotationY -= 0.1f;
        }
        if (key == GLFW_KEY_RIGHT) {
            g_engine->rotationY += 0.1f;
        }
        
        // Scale controls
        if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {  // '+' key
            g_engine->scale *= 1.1f;
        }
        if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {  // '-' key
            g_engine->scale *= 0.9f;
        }
        
        // Reset
        if (key == GLFW_KEY_R) {
            g_engine->rotationX = 0.0f;
            g_engine->rotationY = 0.0f;
            g_engine->rotationZ = 0.0f;
            g_engine->scale = 1.0f;
            std::cout << "Transformations reset" << std::endl;
        }
    }
}

/**
 * @brief Entry point
 */
int main() {
    Engine engine;
    
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return -1;
    }
    
    engine.run();
    
    return 0;
}
