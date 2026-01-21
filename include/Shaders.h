#ifndef SHADERS_H
#define SHADERS_H

#include <glm/glm.hpp>
#include "Rasterizer.h"

/**
 * @brief Light structure for shading calculations
 */
struct Light {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 ambient;
    
    Light() : position(0.0f, 5.0f, 5.0f), 
              color(1.0f, 1.0f, 1.0f),
              ambient(0.2f, 0.2f, 0.2f) {}
};

/**
 * @brief Material properties for objects
 */
struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    
    Material() : ambient(0.2f, 0.2f, 0.2f),
                 diffuse(0.8f, 0.8f, 0.8f),
                 specular(1.0f, 1.0f, 1.0f),
                 shininess(32.0f) {}
};

/**
 * @brief Shaders class implementing lighting and shading models
 * 
 * This class provides implementations of:
 * - Gouraud Shading (per-vertex lighting, interpolated across triangle)
 * - Phong Shading (per-pixel lighting)
 * - Blinn-Phong reflection model
 */
class Shaders {
public:
    // Gouraud Shading: Calculate color at vertex
    // Light calculation is done per-vertex, then interpolated across the triangle
    static Color computeGouraudShading(
        const glm::vec3& vertexPos,      // Vertex position in world space
        const glm::vec3& normal,         // Vertex normal
        const glm::vec3& viewPos,        // Camera position
        const Light& light,              // Light source
        const Material& material         // Material properties
    );
    
    // Phong Shading: Calculate color at each pixel
    // Light calculation is done per-pixel using interpolated normals
    static Color computePhongShading(
        const glm::vec3& fragPos,        // Fragment position in world space
        const glm::vec3& normal,         // Interpolated normal at fragment
        const glm::vec3& viewPos,        // Camera position
        const Light& light,              // Light source
        const Material& material         // Material properties
    );
    
    // Helper function to interpolate colors (for Gouraud shading)
    static Color interpolateColor(const Color& c1, const Color& c2, float t);
    static Color interpolateColor(const Color& c1, const Color& c2, const Color& c3,
                                  float u, float v, float w);
    
    // Helper function to convert glm::vec3 color to Color struct
    static Color vec3ToColor(const glm::vec3& color);
    
private:
    // Blinn-Phong lighting model components
    static glm::vec3 calculateAmbient(const Light& light, const Material& material);
    static glm::vec3 calculateDiffuse(const glm::vec3& lightDir, const glm::vec3& normal,
                                     const Light& light, const Material& material);
    static glm::vec3 calculateSpecular(const glm::vec3& lightDir, const glm::vec3& normal,
                                      const glm::vec3& viewDir, const Light& light,
                                      const Material& material);
};

#endif // SHADERS_H
