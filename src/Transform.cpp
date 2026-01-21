#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>

/**
 * @brief Constructor - Initializes all matrices to identity
 */
Transform::Transform() {
    modelMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::mat4(1.0f);
}

/**
 * @brief Sets the model transformation matrix
 */
void Transform::setModelMatrix(const glm::mat4& model) {
    modelMatrix = model;
}

/**
 * @brief Sets the view transformation matrix
 */
void Transform::setViewMatrix(const glm::mat4& view) {
    viewMatrix = view;
}

/**
 * @brief Sets the projection transformation matrix
 */
void Transform::setProjectionMatrix(const glm::mat4& projection) {
    projectionMatrix = projection;
}

/**
 * @brief Creates a translation matrix using homogeneous coordinates
 * 
 * Translation Matrix (4x4):
 * | 1  0  0  tx |
 * | 0  1  0  ty |
 * | 0  0  1  tz |
 * | 0  0  0  1  |
 */
glm::mat4 Transform::createTranslationMatrix(float x, float y, float z) {
    return glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
}

/**
 * @brief Creates a rotation matrix for rotations around X, Y, and Z axes
 * 
 * Rotation matrices are combined in the order: Z * Y * X
 * Angles are in radians.
 */
glm::mat4 Transform::createRotationMatrix(float angleX, float angleY, float angleZ) {
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    
    // Rotate around X-axis
    if (angleX != 0.0f) {
        rotationMatrix = glm::rotate(rotationMatrix, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    
    // Rotate around Y-axis
    if (angleY != 0.0f) {
        rotationMatrix = glm::rotate(rotationMatrix, angleY, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    
    // Rotate around Z-axis
    if (angleZ != 0.0f) {
        rotationMatrix = glm::rotate(rotationMatrix, angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    return rotationMatrix;
}

/**
 * @brief Creates a scale matrix using homogeneous coordinates
 * 
 * Scale Matrix (4x4):
 * | sx  0   0   0 |
 * | 0   sy  0   0 |
 * | 0   0   sz  0 |
 * | 0   0   0   1 |
 */
glm::mat4 Transform::createScaleMatrix(float sx, float sy, float sz) {
    return glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));
}

/**
 * @brief Sets up the view matrix using the "look-at" method
 * 
 * This creates a view matrix that transforms world space to camera space.
 * 
 * @param eye Camera position in world space
 * @param center Point the camera is looking at
 * @param up Up vector defining camera orientation
 */
void Transform::setLookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) {
    viewMatrix = glm::lookAt(eye, center, up);
}

/**
 * @brief Sets up a perspective projection matrix
 * 
 * This creates a matrix that projects 3D points onto a 2D plane using
 * perspective division (objects farther away appear smaller).
 * 
 * @param fovy Field of view in the Y direction (radians)
 * @param aspect Aspect ratio (width/height)
 * @param near Distance to near clipping plane
 * @param far Distance to far clipping plane
 */
void Transform::setPerspective(float fovy, float aspect, float near, float far) {
    projectionMatrix = glm::perspective(fovy, aspect, near, far);
}

/**
 * @brief Sets up an orthographic projection matrix
 * 
 * This creates a matrix for parallel projection (no perspective).
 * Used for 2D rendering or CAD applications.
 * 
 * @param left Left boundary of viewing volume
 * @param right Right boundary
 * @param bottom Bottom boundary
 * @param top Top boundary
 * @param near Near clipping plane
 * @param far Far clipping plane
 */
void Transform::setOrthographic(float left, float right, float bottom, float top, 
                                float near, float far) {
    projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
}

/**
 * @brief Transforms a vertex through the complete pipeline (MVP)
 * 
 * The transformation pipeline:
 * 1. Model Space -> World Space (Model Matrix)
 * 2. World Space -> View Space (View Matrix)
 * 3. View Space -> Clip Space (Projection Matrix)
 * 4. Clip Space -> NDC (Perspective Division - done outside)
 * 
 * @param vertex Vertex in model space (homogeneous coordinates)
 * @return Transformed vertex in clip space
 */
glm::vec4 Transform::transformVertex(const glm::vec4& vertex) const {
    return projectionMatrix * viewMatrix * modelMatrix * vertex;
}

/**
 * @brief Transforms a normal vector
 * 
 * Normals require special handling - they should be transformed by the
 * inverse transpose of the model-view matrix to maintain perpendicularity.
 * 
 * @param normal Normal vector in model space
 * @return Transformed normal in view space
 */
glm::vec3 Transform::transformNormal(const glm::vec3& normal) const {
    glm::mat3 normalMatrix = getNormalMatrix();
    return glm::normalize(normalMatrix * normal);
}

/**
 * @brief Computes the normal matrix (inverse transpose of model-view matrix)
 */
glm::mat3 Transform::getNormalMatrix() const {
    glm::mat4 modelView = viewMatrix * modelMatrix;
    return glm::transpose(glm::inverse(glm::mat3(modelView)));
}

/**
 * @brief Transforms from NDC to screen coordinates
 * 
 * NDC (Normalized Device Coordinates) range from [-1, 1] in all axes.
 * This function maps them to screen coordinates [0, width] x [0, height].
 * 
 * @param ndcCoord Vertex in NDC after perspective division
 * @param screenWidth Width of the viewport in pixels
 * @param screenHeight Height of the viewport in pixels
 * @return Screen coordinates
 */
glm::vec2 Transform::viewportTransform(const glm::vec4& ndcCoord, 
                                       int screenWidth, int screenHeight) const {
    // Map from [-1, 1] to [0, width] and [0, height]
    float x = (ndcCoord.x + 1.0f) * 0.5f * screenWidth;
    float y = (1.0f - ndcCoord.y) * 0.5f * screenHeight;  // Flip Y (screen Y grows downward)
    
    return glm::vec2(x, y);
}

/**
 * @brief Cohen-Sutherland Line Clipping Algorithm
 * 
 * This algorithm efficiently clips lines to a rectangular viewport.
 * It uses 4-bit region codes to quickly identify and clip line segments.
 * 
 * Region codes:
 *   1001 | 1000 | 1010
 *   -----+------+-----
 *   0001 | 0000 | 0010  <- 0000 is inside the viewport
 *   -----+------+-----
 *   0101 | 0100 | 0110
 * 
 * @param x1, y1 First endpoint (modified if clipped)
 * @param x2, y2 Second endpoint (modified if clipped)
 * @param xMin, yMin, xMax, yMax Clipping rectangle bounds
 * @return true if line is visible after clipping, false if completely outside
 */
bool Transform::clipLine(float& x1, float& y1, float& x2, float& y2,
                        float xMin, float yMin, float xMax, float yMax) {
    // Compute region codes for both endpoints
    int code1 = computeOutCode(x1, y1, xMin, yMin, xMax, yMax);
    int code2 = computeOutCode(x2, y2, xMin, yMin, xMax, yMax);
    
    bool accept = false;
    
    while (true) {
        if ((code1 | code2) == 0) {
            // Both endpoints inside rectangle - trivially accept
            accept = true;
            break;
        } else if (code1 & code2) {
            // Both endpoints share an outside region - trivially reject
            break;
        } else {
            // Line needs clipping
            // Pick an endpoint that is outside
            int codeOut = code1 ? code1 : code2;
            
            float x, y;
            
            // Find intersection point using formulas:
            // y = y1 + slope * (x - x1), x = x1 + (1/slope) * (y - y1)
            if (codeOut & TOP) {           // Point is above the clip rectangle
                x = x1 + (x2 - x1) * (yMax - y1) / (y2 - y1);
                y = yMax;
            } else if (codeOut & BOTTOM) { // Point is below the clip rectangle
                x = x1 + (x2 - x1) * (yMin - y1) / (y2 - y1);
                y = yMin;
            } else if (codeOut & RIGHT) {  // Point is to the right
                y = y1 + (y2 - y1) * (xMax - x1) / (x2 - x1);
                x = xMax;
            } else if (codeOut & LEFT) {   // Point is to the left
                y = y1 + (y2 - y1) * (xMin - x1) / (x2 - x1);
                x = xMin;
            }
            
            // Replace the outside point with the intersection point
            if (codeOut == code1) {
                x1 = x;
                y1 = y;
                code1 = computeOutCode(x1, y1, xMin, yMin, xMax, yMax);
            } else {
                x2 = x;
                y2 = y;
                code2 = computeOutCode(x2, y2, xMin, yMin, xMax, yMax);
            }
        }
    }
    
    return accept;
}

/**
 * @brief Computes the region code for a point
 * 
 * Region code is a 4-bit code indicating which region the point is in:
 * Bit 0 (LEFT):   point is to the left of clip window
 * Bit 1 (RIGHT):  point is to the right
 * Bit 2 (BOTTOM): point is below
 * Bit 3 (TOP):    point is above
 */
int Transform::computeOutCode(float x, float y, float xMin, float yMin, 
                             float xMax, float yMax) const {
    int code = INSIDE;  // 0000
    
    if (x < xMin) {
        code |= LEFT;   // 0001
    } else if (x > xMax) {
        code |= RIGHT;  // 0010
    }
    
    if (y < yMin) {
        code |= BOTTOM; // 0100
    } else if (y > yMax) {
        code |= TOP;    // 1000
    }
    
    return code;
}

/**
 * @brief Pushes the current model matrix onto the stack
 * 
 * Useful for hierarchical transformations (e.g., robot arm, scene graph)
 */
void Transform::pushMatrix() {
    matrixStack.push_back(modelMatrix);
}

/**
 * @brief Pops the top matrix from the stack and sets it as the model matrix
 */
void Transform::popMatrix() {
    if (!matrixStack.empty()) {
        modelMatrix = matrixStack.back();
        matrixStack.pop_back();
    }
}
