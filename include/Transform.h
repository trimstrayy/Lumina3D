#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

/**
 * @brief Clipping result codes for Cohen-Sutherland algorithm
 */
enum ClipCode {
    INSIDE = 0,  // 0000
    LEFT = 1,    // 0001
    RIGHT = 2,   // 0010
    BOTTOM = 4,  // 0100
    TOP = 8      // 1000
};

/**
 * @brief Transform class handling the graphics pipeline transformations
 * 
 * This class implements:
 * - Model, View, and Projection matrices using homogeneous coordinates
 * - Cohen-Sutherland line clipping algorithm
 * - Viewport transformations
 * - Matrix stack operations
 */
class Transform {
public:
    Transform();
    
    // Matrix operations
    void setModelMatrix(const glm::mat4& model);
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& projection);
    
    // Helper functions to create transformation matrices
    glm::mat4 createTranslationMatrix(float x, float y, float z);
    glm::mat4 createRotationMatrix(float angleX, float angleY, float angleZ);
    glm::mat4 createScaleMatrix(float sx, float sy, float sz);
    
    // Camera setup
    void setLookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);
    void setPerspective(float fovy, float aspect, float near, float far);
    void setOrthographic(float left, float right, float bottom, float top, 
                        float near, float far);
    
    // Transformation pipeline
    glm::vec4 transformVertex(const glm::vec4& vertex) const;
    glm::vec3 transformNormal(const glm::vec3& normal) const;
    
    // Viewport transformation (NDC to screen coordinates)
    glm::vec2 viewportTransform(const glm::vec4& ndcCoord, int screenWidth, 
                                int screenHeight) const;
    
    // Cohen-Sutherland Line Clipping Algorithm
    bool clipLine(float& x1, float& y1, float& x2, float& y2, 
                  float xMin, float yMin, float xMax, float yMax);
    
    // Getters
    const glm::mat4& getModelMatrix() const { return modelMatrix; }
    const glm::mat4& getViewMatrix() const { return viewMatrix; }
    const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
    glm::mat4 getMVPMatrix() const { return projectionMatrix * viewMatrix * modelMatrix; }
    glm::mat4 getModelViewMatrix() const { return viewMatrix * modelMatrix; }
    glm::mat3 getNormalMatrix() const;
    
    // Matrix stack operations (useful for hierarchical transformations)
    void pushMatrix();
    void popMatrix();
    
private:
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    std::vector<glm::mat4> matrixStack;
    
    // Helper for Cohen-Sutherland
    int computeOutCode(float x, float y, float xMin, float yMin, 
                      float xMax, float yMax) const;
};

#endif // TRANSFORM_H
