#include "Shaders.h"
#include <algorithm>
#include <cmath>

/**
 * @brief Gouraud Shading Implementation
 * 
 * Gouraud shading computes lighting at each vertex, then interpolates
 * the colors across the triangle. This is less expensive than Phong shading
 * but can produce less accurate results, especially for specular highlights.
 * 
 * Process:
 * 1. Calculate lighting at each vertex
 * 2. Store resulting colors in vertex structure
 * 3. Interpolate colors during rasterization
 * 
 * @return Color computed using Blinn-Phong reflection model
 */
Color Shaders::computeGouraudShading(
    const glm::vec3& vertexPos,
    const glm::vec3& normal,
    const glm::vec3& viewPos,
    const Light& light,
    const Material& material
) {
    // Normalize the normal vector
    glm::vec3 norm = glm::normalize(normal);
    
    // Calculate light direction
    glm::vec3 lightDir = glm::normalize(light.position - vertexPos);
    
    // Calculate view direction
    glm::vec3 viewDir = glm::normalize(viewPos - vertexPos);
    
    // Compute lighting components using Blinn-Phong model
    glm::vec3 ambient = calculateAmbient(light, material);
    glm::vec3 diffuse = calculateDiffuse(lightDir, norm, light, material);
    glm::vec3 specular = calculateSpecular(lightDir, norm, viewDir, light, material);
    
    // Combine all lighting components
    glm::vec3 finalColor = ambient + diffuse + specular;
    
    // Clamp to [0, 1] range
    finalColor = glm::clamp(finalColor, 0.0f, 1.0f);
    
    return vec3ToColor(finalColor);
}

/**
 * @brief Phong Shading Implementation
 * 
 * Phong shading interpolates the normal vectors across the triangle
 * and computes lighting at each pixel. This produces more accurate
 * lighting, especially for specular highlights, but is more expensive.
 * 
 * Process:
 * 1. Interpolate vertex normals across triangle
 * 2. Calculate lighting at each fragment (pixel)
 * 3. More accurate but computationally expensive
 * 
 * @return Color computed using Blinn-Phong reflection model
 */
Color Shaders::computePhongShading(
    const glm::vec3& fragPos,
    const glm::vec3& normal,
    const glm::vec3& viewPos,
    const Light& light,
    const Material& material
) {
    // Same calculation as Gouraud, but done per-pixel instead of per-vertex
    glm::vec3 norm = glm::normalize(normal);
    glm::vec3 lightDir = glm::normalize(light.position - fragPos);
    glm::vec3 viewDir = glm::normalize(viewPos - fragPos);
    
    glm::vec3 ambient = calculateAmbient(light, material);
    glm::vec3 diffuse = calculateDiffuse(lightDir, norm, light, material);
    glm::vec3 specular = calculateSpecular(lightDir, norm, viewDir, light, material);
    
    glm::vec3 finalColor = ambient + diffuse + specular;
    finalColor = glm::clamp(finalColor, 0.0f, 1.0f);
    
    return vec3ToColor(finalColor);
}

/**
 * @brief Calculates the ambient lighting component
 * 
 * Ambient light is uniform illumination that doesn't depend on
 * light or view direction. It simulates indirect lighting.
 * 
 * Formula: ambient = light.ambient * material.ambient
 */
glm::vec3 Shaders::calculateAmbient(const Light& light, const Material& material) {
    return light.ambient * material.ambient;
}

/**
 * @brief Calculates the diffuse lighting component
 * 
 * Diffuse reflection represents light scattered equally in all directions.
 * It depends on the angle between the surface normal and light direction.
 * 
 * Formula: diffuse = light.color * material.diffuse * max(dot(N, L), 0)
 * where N = normal, L = light direction
 */
glm::vec3 Shaders::calculateDiffuse(const glm::vec3& lightDir, const glm::vec3& normal,
                                   const Light& light, const Material& material) {
    // Lambert's cosine law: intensity proportional to cos(angle)
    float diff = std::max(glm::dot(normal, lightDir), 0.0f);
    return light.color * material.diffuse * diff;
}

/**
 * @brief Calculates the specular lighting component using Blinn-Phong model
 * 
 * Specular reflection creates highlights. The Blinn-Phong model uses the
 * halfway vector between light and view direction for efficiency.
 * 
 * Formula: specular = light.color * material.specular * pow(max(dot(N, H), 0), shininess)
 * where H = normalize(L + V) is the halfway vector
 */
glm::vec3 Shaders::calculateSpecular(const glm::vec3& lightDir, const glm::vec3& normal,
                                    const glm::vec3& viewDir, const Light& light,
                                    const Material& material) {
    // Blinn-Phong: use halfway vector instead of reflection vector
    glm::vec3 halfwayDir = glm::normalize(lightDir + viewDir);
    
    // Calculate specular intensity
    float spec = std::pow(std::max(glm::dot(normal, halfwayDir), 0.0f), material.shininess);
    
    return light.color * material.specular * spec;
}

/**
 * @brief Linearly interpolates between two colors
 * 
 * @param c1 First color
 * @param c2 Second color
 * @param t Interpolation parameter [0, 1]
 * @return Interpolated color
 */
Color Shaders::interpolateColor(const Color& c1, const Color& c2, float t) {
    return Color(
        static_cast<uint8_t>(c1.r * (1 - t) + c2.r * t),
        static_cast<uint8_t>(c1.g * (1 - t) + c2.g * t),
        static_cast<uint8_t>(c1.b * (1 - t) + c2.b * t),
        static_cast<uint8_t>(c1.a * (1 - t) + c2.a * t)
    );
}

/**
 * @brief Interpolates between three colors using barycentric coordinates
 * 
 * @param c1, c2, c3 Colors at triangle vertices
 * @param u, v, w Barycentric coordinates (should sum to 1.0)
 * @return Interpolated color
 */
Color Shaders::interpolateColor(const Color& c1, const Color& c2, const Color& c3,
                                float u, float v, float w) {
    return Color(
        static_cast<uint8_t>(c1.r * u + c2.r * v + c3.r * w),
        static_cast<uint8_t>(c1.g * u + c2.g * v + c3.g * w),
        static_cast<uint8_t>(c1.b * u + c2.b * v + c3.b * w),
        static_cast<uint8_t>(c1.a * u + c2.a * v + c3.a * w)
    );
}

/**
 * @brief Converts a glm::vec3 color (range [0,1]) to Color struct (range [0,255])
 */
Color Shaders::vec3ToColor(const glm::vec3& color) {
    return Color(
        static_cast<uint8_t>(color.r * 255.0f),
        static_cast<uint8_t>(color.g * 255.0f),
        static_cast<uint8_t>(color.b * 255.0f),
        255
    );
}
