#include "Rasterizer.h"
#include <algorithm>
#include <cmath>
#include <cstring>

/**
 * @brief Constructor - Initializes frame buffer and depth buffer
 */
Rasterizer::Rasterizer(int width, int height) 
    : width(width), height(height) {
    // Allocate frame buffer (RGB format: 3 bytes per pixel)
    frameBuffer = new uint8_t[width * height * 3];
    
    // Allocate depth buffer (1 float per pixel)
    depthBuffer = new float[width * height];
    
    // Initialize buffers
    clearBuffers();
}

/**
 * @brief Destructor - Cleans up allocated memory
 */
Rasterizer::~Rasterizer() {
    delete[] frameBuffer;
    delete[] depthBuffer;
}

/**
 * @brief Clears both frame buffer and depth buffer
 */
void Rasterizer::clearBuffers(const Color& clearColor) {
    // Clear frame buffer with specified color
    for (int i = 0; i < width * height; ++i) {
        frameBuffer[i * 3 + 0] = clearColor.r;
        frameBuffer[i * 3 + 1] = clearColor.g;
        frameBuffer[i * 3 + 2] = clearColor.b;
    }
    
    // Clear depth buffer with maximum depth (far plane)
    for (int i = 0; i < width * height; ++i) {
        depthBuffer[i] = 1.0f;  // 1.0 = far plane in normalized coordinates
    }
}

/**
 * @brief Bresenham's Line Algorithm - Draws a line between two points
 * 
 * This algorithm uses only integer arithmetic to efficiently rasterize lines.
 * It works by determining which pixels best approximate the ideal line.
 * 
 * Time Complexity: O(max(|x2-x1|, |y2-y1|))
 * Space Complexity: O(1)
 */
void Rasterizer::draw_line(int x1, int y1, int x2, int y2, const Color& color) {
    // Determine if the line is steep (more vertical than horizontal)
    if (std::abs(y2 - y1) < std::abs(x2 - x1)) {
        // Line is more horizontal
        if (x1 > x2) {
            drawLineLow(x2, y2, x1, y1, color);
        } else {
            drawLineLow(x1, y1, x2, y2, color);
        }
    } else {
        // Line is more vertical
        if (y1 > y2) {
            drawLineHigh(x2, y2, x1, y1, color);
        } else {
            drawLineHigh(x1, y1, x2, y2, color);
        }
    }
}

/**
 * @brief Helper for Bresenham's algorithm when slope is < 1
 */
void Rasterizer::drawLineLow(int x1, int y1, int x2, int y2, const Color& color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int yi = 1;
    
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    
    // Decision parameter for determining which pixel to draw
    int D = 2 * dy - dx;
    int y = y1;
    
    for (int x = x1; x <= x2; ++x) {
        setPixel(x, y, color);
        
        if (D > 0) {
            y += yi;
            D -= 2 * dx;
        }
        D += 2 * dy;
    }
}

/**
 * @brief Helper for Bresenham's algorithm when slope is >= 1
 */
void Rasterizer::drawLineHigh(int x1, int y1, int x2, int y2, const Color& color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int xi = 1;
    
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }
    
    // Decision parameter
    int D = 2 * dx - dy;
    int x = x1;
    
    for (int y = y1; y <= y2; ++y) {
        setPixel(x, y, color);
        
        if (D > 0) {
            x += xi;
            D -= 2 * dy;
        }
        D += 2 * dx;
    }
}

/**
 * @brief Mid-Point Circle Algorithm - Draws a circle
 * 
 * This algorithm uses the 8-way symmetry of a circle to draw all 8 octants
 * by calculating only one octant. It uses the mid-point decision parameter
 * to determine which pixel to select.
 * 
 * Time Complexity: O(r) where r is the radius
 * Space Complexity: O(1)
 */
void Rasterizer::draw_circle(int xc, int yc, int r, const Color& color) {
    int x = 0;
    int y = r;
    
    // Initial decision parameter
    // P = 1 - r (optimized to avoid floating point)
    int d = 1 - r;
    
    // Draw initial points using 8-way symmetry
    drawCirclePoints(xc, yc, x, y, color);
    
    while (x < y) {
        x++;
        
        // Mid-point is inside or on the perimeter
        if (d < 0) {
            // Select pixel closer to the circle (E direction)
            d = d + 2 * x + 1;
        } else {
            // Select pixel in SE direction
            y--;
            d = d + 2 * (x - y) + 1;
        }
        
        // Draw all 8 symmetric points
        drawCirclePoints(xc, yc, x, y, color);
    }
}

/**
 * @brief Helper function to draw all 8 symmetric points of a circle
 */
void Rasterizer::drawCirclePoints(int xc, int yc, int x, int y, const Color& color) {
    setPixel(xc + x, yc + y, color);  // Octant 1
    setPixel(xc - x, yc + y, color);  // Octant 2
    setPixel(xc + x, yc - y, color);  // Octant 8
    setPixel(xc - x, yc - y, color);  // Octant 7
    setPixel(xc + y, yc + x, color);  // Octant 3
    setPixel(xc - y, yc + x, color);  // Octant 4
    setPixel(xc + y, yc - x, color);  // Octant 6
    setPixel(xc - y, yc - x, color);  // Octant 5
}

/**
 * @brief Draws a filled triangle with shading
 * 
 * This method implements scanline rasterization with Z-buffering.
 * It splits the triangle into flat-top and flat-bottom triangles for easier processing.
 * 
 * @param useGouraud If true, uses Gouraud shading (vertex colors interpolated)
 *                   If false, uses flat shading
 */
void Rasterizer::drawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, 
                             bool useGouraud) {
    // Sort vertices by y-coordinate (v1.y <= v2.y <= v3.y)
    std::vector<Vertex> verts = {v1, v2, v3};
    std::sort(verts.begin(), verts.end(), [](const Vertex& a, const Vertex& b) {
        return a.position.y < b.position.y;
    });
    
    const Vertex& top = verts[0];
    const Vertex& mid = verts[1];
    const Vertex& bot = verts[2];
    
    // Check for degenerate triangle
    if (top.position.y == bot.position.y) return;
    
    // Check if we need to split the triangle
    if (mid.position.y == bot.position.y) {
        // Natural flat-bottom triangle
        fillFlatBottomTriangle(top, mid, bot, useGouraud);
    } else if (top.position.y == mid.position.y) {
        // Natural flat-top triangle
        fillFlatTopTriangle(top, mid, bot, useGouraud);
    } else {
        // Split into flat-bottom and flat-top triangles
        // Find the point on edge top-bot that has the same y as mid
        float t = (mid.position.y - top.position.y) / (bot.position.y - top.position.y);
        
        Vertex splitVertex;
        splitVertex.position = glm::mix(top.position, bot.position, t);
        splitVertex.worldPos = glm::mix(top.worldPos, bot.worldPos, t);
        splitVertex.normal = glm::normalize(glm::mix(top.normal, bot.normal, t));
        splitVertex.color = Color(
            static_cast<uint8_t>(top.color.r * (1 - t) + bot.color.r * t),
            static_cast<uint8_t>(top.color.g * (1 - t) + bot.color.g * t),
            static_cast<uint8_t>(top.color.b * (1 - t) + bot.color.b * t)
        );
        
        // Draw both sub-triangles
        fillFlatBottomTriangle(top, mid, splitVertex, useGouraud);
        fillFlatTopTriangle(mid, splitVertex, bot, useGouraud);
    }
}

/**
 * @brief Fills a flat-bottom triangle using scanline rasterization
 */
void Rasterizer::fillFlatBottomTriangle(const Vertex& v1, const Vertex& v2, 
                                       const Vertex& v3, bool useGouraud) {
    float invSlope1 = (v2.position.x - v1.position.x) / (v2.position.y - v1.position.y);
    float invSlope2 = (v3.position.x - v1.position.x) / (v3.position.y - v1.position.y);
    
    float x1 = v1.position.x;
    float x2 = v1.position.x;
    
    int startY = static_cast<int>(std::ceil(v1.position.y));
    int endY = static_cast<int>(std::ceil(v2.position.y));
    
    for (int y = startY; y < endY; ++y) {
        int xStart = static_cast<int>(std::ceil(x1));
        int xEnd = static_cast<int>(std::ceil(x2));
        
        for (int x = xStart; x < xEnd; ++x) {
            // Calculate barycentric coordinates for interpolation
            glm::vec3 bary = computeBarycentric(
                static_cast<float>(x), static_cast<float>(y),
                glm::vec2(v1.position.x, v1.position.y),
                glm::vec2(v2.position.x, v2.position.y),
                glm::vec2(v3.position.x, v3.position.y)
            );
            
            // Interpolate depth
            float depth = bary.x * v1.position.z + bary.y * v2.position.z + bary.z * v3.position.z;
            
            // Interpolate color
            Color color(
                static_cast<uint8_t>(bary.x * v1.color.r + bary.y * v2.color.r + bary.z * v3.color.r),
                static_cast<uint8_t>(bary.x * v1.color.g + bary.y * v2.color.g + bary.z * v3.color.g),
                static_cast<uint8_t>(bary.x * v1.color.b + bary.y * v2.color.b + bary.z * v3.color.b)
            );
            
            setPixelWithDepth(x, y, depth, color);
        }
        
        x1 += invSlope1;
        x2 += invSlope2;
    }
}

/**
 * @brief Fills a flat-top triangle using scanline rasterization
 */
void Rasterizer::fillFlatTopTriangle(const Vertex& v1, const Vertex& v2, 
                                    const Vertex& v3, bool useGouraud) {
    float invSlope1 = (v3.position.x - v1.position.x) / (v3.position.y - v1.position.y);
    float invSlope2 = (v3.position.x - v2.position.x) / (v3.position.y - v2.position.y);
    
    float x1 = v3.position.x;
    float x2 = v3.position.x;
    
    int startY = static_cast<int>(std::ceil(v3.position.y));
    int endY = static_cast<int>(std::ceil(v1.position.y));
    
    for (int y = startY; y > endY; --y) {
        int xStart = static_cast<int>(std::ceil(x1));
        int xEnd = static_cast<int>(std::ceil(x2));
        
        for (int x = xStart; x < xEnd; ++x) {
            glm::vec3 bary = computeBarycentric(
                static_cast<float>(x), static_cast<float>(y),
                glm::vec2(v1.position.x, v1.position.y),
                glm::vec2(v2.position.x, v2.position.y),
                glm::vec2(v3.position.x, v3.position.y)
            );
            
            float depth = bary.x * v1.position.z + bary.y * v2.position.z + bary.z * v3.position.z;
            
            Color color(
                static_cast<uint8_t>(bary.x * v1.color.r + bary.y * v2.color.r + bary.z * v3.color.r),
                static_cast<uint8_t>(bary.x * v1.color.g + bary.y * v2.color.g + bary.z * v3.color.g),
                static_cast<uint8_t>(bary.x * v1.color.b + bary.y * v2.color.b + bary.z * v3.color.b)
            );
            
            setPixelWithDepth(x, y, depth, color);
        }
        
        x1 -= invSlope1;
        x2 -= invSlope2;
    }
}

/**
 * @brief Draws a wireframe triangle
 */
void Rasterizer::drawWireframeTriangle(const Vertex& v1, const Vertex& v2, 
                                      const Vertex& v3, const Color& color) {
    int x1 = static_cast<int>(v1.position.x);
    int y1 = static_cast<int>(v1.position.y);
    int x2 = static_cast<int>(v2.position.x);
    int y2 = static_cast<int>(v2.position.y);
    int x3 = static_cast<int>(v3.position.x);
    int y3 = static_cast<int>(v3.position.y);
    
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x3, y3, color);
    draw_line(x3, y3, x1, y1, color);
}

/**
 * @brief Computes barycentric coordinates for point (x,y) in triangle (v1,v2,v3)
 * 
 * Barycentric coordinates allow us to interpolate values across a triangle.
 * They represent the weights of each vertex for a point inside the triangle.
 */
glm::vec3 Rasterizer::computeBarycentric(float x, float y, const glm::vec2& v1, 
                                         const glm::vec2& v2, const glm::vec2& v3) {
    float denominator = (v2.y - v3.y) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.y - v3.y);
    
    if (std::abs(denominator) < 1e-6f) {
        return glm::vec3(1.0f / 3.0f);  // Degenerate triangle
    }
    
    float w1 = ((v2.y - v3.y) * (x - v3.x) + (v3.x - v2.x) * (y - v3.y)) / denominator;
    float w2 = ((v3.y - v1.y) * (x - v3.x) + (v1.x - v3.x) * (y - v3.y)) / denominator;
    float w3 = 1.0f - w1 - w2;
    
    return glm::vec3(w1, w2, w3);
}

/**
 * @brief Sets a pixel in the frame buffer
 */
void Rasterizer::setPixel(int x, int y, const Color& color) {
    if (!isInBounds(x, y)) return;
    
    int index = (y * width + x) * 3;
    frameBuffer[index + 0] = color.r;
    frameBuffer[index + 1] = color.g;
    frameBuffer[index + 2] = color.b;
}

/**
 * @brief Sets a pixel with Z-buffer depth testing
 * 
 * This implements the Z-buffer algorithm for hidden surface removal.
 * Only draws the pixel if it's closer to the camera than the current depth value.
 */
void Rasterizer::setPixelWithDepth(int x, int y, float depth, const Color& color) {
    if (!isInBounds(x, y)) return;
    
    int index = y * width + x;
    
    // Depth test: only draw if closer to camera
    if (depth < depthBuffer[index]) {
        depthBuffer[index] = depth;
        
        int colorIndex = index * 3;
        frameBuffer[colorIndex + 0] = color.r;
        frameBuffer[colorIndex + 1] = color.g;
        frameBuffer[colorIndex + 2] = color.b;
    }
}

/**
 * @brief Checks if pixel coordinates are within bounds
 */
bool Rasterizer::isInBounds(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}
