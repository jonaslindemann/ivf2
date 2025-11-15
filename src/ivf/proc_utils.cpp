#include <ivf/proc_utils.h>
#include <cmath>
#include <algorithm>

using namespace ivf;

namespace {
    // Permutation table for noise generation
    static const int PERM_SIZE = 256;
    static int perm[PERM_SIZE * 2];
    static bool permInitialized = false;

    void initPerm() {
        if (permInitialized) return;
        
        // Initialize permutation table with a fixed pattern for consistency
        int p[PERM_SIZE];
        for (int i = 0; i < PERM_SIZE; i++) {
            p[i] = i;
        }
        
        // Simple shuffle with fixed seed for reproducibility
        for (int i = PERM_SIZE - 1; i > 0; i--) {
            int j = (i * 75 + 23) % (i + 1);
            std::swap(p[i], p[j]);
        }
        
        // Duplicate for wrapping
        for (int i = 0; i < PERM_SIZE * 2; i++) {
            perm[i] = p[i % PERM_SIZE];
        }
        
        permInitialized = true;
    }
    
    // Gradient vectors for 2D noise
    float grad2(int hash, float x, float y) {
        int h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }
}

float ProcUtils::hash(int x, int y)
{
    initPerm();
    int h = perm[(perm[x & 255] + y) & 255];
    return h / 255.0f;
}

float ProcUtils::smoothstep(float t)
{
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

float ProcUtils::bilerp(float a, float b, float c, float d, float tx, float ty)
{
    float stx = smoothstep(tx);
    float sty = smoothstep(ty);
    float ab = a + stx * (b - a);
    float cd = c + stx * (d - c);
    return ab + sty * (cd - ab);
}

float ProcUtils::noise(float x, float y)
{
    initPerm();
    
    // Integer part
    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;
    
    // Fractional part
    float xf = x - std::floor(x);
    float yf = y - std::floor(y);
    
    // Get gradient indices
    int aa = perm[perm[xi] + yi];
    int ab = perm[perm[xi] + yi + 1];
    int ba = perm[perm[xi + 1] + yi];
    int bb = perm[perm[xi + 1] + yi + 1];
    
    // Calculate gradients
    float g1 = grad2(aa, xf, yf);
    float g2 = grad2(ba, xf - 1.0f, yf);
    float g3 = grad2(ab, xf, yf - 1.0f);
    float g4 = grad2(bb, xf - 1.0f, yf - 1.0f);
    
    // Interpolate
    return bilerp(g1, g2, g3, g4, xf, yf);
}

float ProcUtils::fbm(float x, float y, int octaves, float persistence, float lacunarity)
{
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; i++) {
        total += noise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}

float ProcUtils::voronoi(float x, float y, float cellSize)
{
    float scaledX = x / cellSize;
    float scaledY = y / cellSize;
    
    int cellX = static_cast<int>(std::floor(scaledX));
    int cellY = static_cast<int>(std::floor(scaledY));
    
    float minDist = 1000.0f;
    
    // Check neighboring cells
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = cellX + dx;
            int ny = cellY + dy;
            
            // Get pseudo-random point in this cell
            float px = nx + hash(nx, ny);
            float py = ny + hash(ny, nx);
            
            // Calculate distance
            float distX = scaledX - px;
            float distY = scaledY - py;
            float dist = std::sqrt(distX * distX + distY * distY);
            
            minDist = std::min(minDist, dist);
        }
    }
    
    return minDist;
}

float ProcUtils::turbulence(float x, float y, int octaves)
{
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    
    for (int i = 0; i < octaves; i++) {
        total += std::abs(noise(x * frequency, y * frequency)) * amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    
    return total;
}

float ProcUtils::marble(float x, float y, float scale, int octaves)
{
    float n = fbm(x * scale, y * scale, octaves);
    float marble = std::sin((x * scale + n * 2.0f) * 3.14159f);
    return (marble + 1.0f) * 0.5f;
}

float ProcUtils::wood(float x, float y, float scale, float rings)
{
    float dist = std::sqrt(x * x + y * y) * scale;
    float n = noise(x * 5.0f, y * 5.0f) * 0.3f;
    float wood = std::sin((dist + n) * rings * 3.14159f);
    return (wood + 1.0f) * 0.5f;
}

float ProcUtils::checkerboard(float x, float y, float size)
{
    int xi = static_cast<int>(std::floor(x / size));
    int yi = static_cast<int>(std::floor(y / size));
    return ((xi + yi) & 1) ? 1.0f : 0.0f;
}

float ProcUtils::gradient(float x, float y, float angle)
{
    float dx = std::cos(angle);
    float dy = std::sin(angle);
    float t = x * dx + y * dy;
    return clamp01(t);
}

float ProcUtils::radialGradient(float x, float y, float centerX, float centerY)
{
    float dx = x - centerX;
    float dy = y - centerY;
    float dist = std::sqrt(dx * dx + dy * dy);
    return clamp01(dist);
}

glm::vec4 ProcUtils::mixColors(const glm::vec4 &color1, const glm::vec4 &color2, float factor)
{
    factor = clamp01(factor);
    return color1 * (1.0f - factor) + color2 * factor;
}

float ProcUtils::clamp01(float value)
{
    return std::max(0.0f, std::min(1.0f, value));
}

float ProcUtils::remap(float value, float inMin, float inMax, float outMin, float outMax)
{
    float t = (value - inMin) / (inMax - inMin);
    return outMin + t * (outMax - outMin);
}
