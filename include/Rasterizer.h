#ifndef RASTERIZER_H
#define RASTERIZER_H

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

/**
 * @brief Structure to represent a color in RGBA format
 */
struct Color {
    uint8_t r, g, b, a;
    
    Color(uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

/**
 * @brief Structure to represent a vertex with position, color, and normal
 */
struct Vertex {
    glm::vec4 position;     // Homogeneous coordinates (x, y, z, w)
    glm::vec3 worldPos;     // World space position for lighting
    glm::vec3 normal;       // Normal vector for lighting calculations
    Color color;            // Vertex color
    
    Vertex() : position(0.0f), worldPos(0.0f), normal(0.0f, 0.0f, 1.0f), color() {}
};

/**
 * @brief Rasterizer class implementing manual drawing algorithms
 * 
 * This class provides manual implementations of:
 * - Bresenham's Line Algorithm
 * - Mid-Point Circle Algorithm
 * - Triangle Rasterization
 * - Z-Buffer (Depth Buffer) management
 */
class Rasterizer {
public:
    Rasterizer(int width, int height);
    ~Rasterizer();
    
    // Buffer management
    void clearBuffers(const Color& clearColor = Color(0, 0, 0, 255));
    uint8_t* getFrameBuffer() const { return frameBuffer; }
    
    // Basic drawing primitives (manually implemented)
    void draw_line(int x1, int y1, int x2, int y2, const Color& color);
    void draw_circle(int xc, int yc, int r, const Color& color);
    
    // Advanced drawing
    void drawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, 
                     bool useGouraud = true);
    void drawWireframeTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, 
                              const Color& color);
    
    // Pixel operations
    void setPixel(int x, int y, const Color& color);
    void setPixelWithDepth(int x, int y, float depth, const Color& color);
    
    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
private:
    int width;
    int height;
    uint8_t* frameBuffer;    // RGB frame buffer (width * height * 3)
    float* depthBuffer;      // Z-buffer for depth testing
    
    // Helper methods for Bresenham's algorithm
    void drawLineLow(int x1, int y1, int x2, int y2, const Color& color);
    void drawLineHigh(int x1, int y1, int x2, int y2, const Color& color);
    
    // Helper method for circle drawing (8-way symmetry)
    void drawCirclePoints(int xc, int yc, int x, int y, const Color& color);
    
    // Triangle rasterization helper
    void fillFlatTopTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, 
                            bool useGouraud);
    void fillFlatBottomTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, 
                               bool useGouraud);
    
    // Barycentric coordinate helper for interpolation
    glm::vec3 computeBarycentric(float x, float y, const glm::vec2& v1, 
                                 const glm::vec2& v2, const glm::vec2& v3);
    
    // Bounds checking
    bool isInBounds(int x, int y) const;
};

#endif // RASTERIZER_H
