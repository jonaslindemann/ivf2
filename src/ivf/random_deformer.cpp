#include <ivf/random_deformer.h>
#include <cmath>
#include <algorithm>

using namespace ivf;

RandomDeformer::RandomDeformer(NoiseType noiseType, DeformationMode mode)
    : m_noiseType(noiseType), m_mode(mode), m_intensity(1.0f), m_frequency(1.0f),
      m_offset(0.0f), m_direction(0, 1, 0), m_center(0.0f), m_time(0.0f),
      m_seed(12345), m_octaves(4), m_persistence(0.5f), m_lacunarity(2.0f),
      m_rng(m_seed), m_uniformDist(-1.0f, 1.0f), m_gaussianDist(0.0f, 1.0f),
      m_cacheValid(false) {}

std::shared_ptr<RandomDeformer> RandomDeformer::create(NoiseType noiseType, DeformationMode mode) {
    return std::make_shared<RandomDeformer>(noiseType, mode);
}

void RandomDeformer::setNoiseType(NoiseType type) {
    m_noiseType = type;
    invalidateCache();
}

RandomDeformer::NoiseType RandomDeformer::noiseType() const {
    return m_noiseType;
}

void RandomDeformer::setDeformationMode(DeformationMode mode) {
    m_mode = mode;
}

RandomDeformer::DeformationMode RandomDeformer::deformationMode() const {
    return m_mode;
}

void RandomDeformer::setIntensity(const glm::vec3& intensity) {
    m_intensity = intensity;
    invalidateCache();
}

void RandomDeformer::setIntensity(float uniformIntensity) {
    m_intensity = glm::vec3(uniformIntensity);
    invalidateCache();
}

glm::vec3 RandomDeformer::intensity() const {
    return m_intensity;
}

void RandomDeformer::setFrequency(const glm::vec3& frequency) {
    m_frequency = frequency;
    invalidateCache();
}

void RandomDeformer::setFrequency(float uniformFrequency) {
    m_frequency = glm::vec3(uniformFrequency);
    invalidateCache();
}

glm::vec3 RandomDeformer::frequency() const {
    return m_frequency;
}

void RandomDeformer::setOffset(const glm::vec3& offset) {
    m_offset = offset;
    invalidateCache();
}

glm::vec3 RandomDeformer::offset() const {
    return m_offset;
}

void RandomDeformer::setDirection(const glm::vec3& direction) {
    m_direction = glm::normalize(direction);
}

glm::vec3 RandomDeformer::direction() const {
    return m_direction;
}

void RandomDeformer::setCenter(const glm::vec3& center) {
    m_center = center;
}

glm::vec3 RandomDeformer::center() const {
    return m_center;
}

void RandomDeformer::setTime(float time) {
    if (std::abs(m_time - time) > 0.001f) {
        m_time = time;
        invalidateCache();
    }
}

float RandomDeformer::time() const {
    return m_time;
}

void RandomDeformer::setSeed(unsigned int seed) {
    m_seed = seed;
    m_rng.seed(seed);
    invalidateCache();
}

unsigned int RandomDeformer::seed() const {
    return m_seed;
}

void RandomDeformer::setOctaves(int octaves) {
    m_octaves = std::max(1, octaves);
    invalidateCache();
}

int RandomDeformer::octaves() const {
    return m_octaves;
}

void RandomDeformer::setPersistence(float persistence) {
    m_persistence = persistence;
    invalidateCache();
}

float RandomDeformer::persistence() const {
    return m_persistence;
}

void RandomDeformer::setLacunarity(float lacunarity) {
    m_lacunarity = lacunarity;
    invalidateCache();
}

float RandomDeformer::lacunarity() const {
    return m_lacunarity;
}

void RandomDeformer::invalidateCache() {
    m_cacheValid = false;
}

void RandomDeformer::regenerateNoise() {
    invalidateCache();
}

void RandomDeformer::apply() {
    if (!m_enabled || !m_originalVertices || !m_deformedVertices) return;
    
    GLuint numVertices = m_originalVertices->rows();
    
    // Ensure cache is the right size
    if (m_noiseCache.size() != numVertices) {
        m_noiseCache.resize(numVertices);
        m_cacheValid = false;
    }
    
    // Regenerate noise if cache is invalid
    if (!m_cacheValid) {
        for (GLuint i = 0; i < numVertices; ++i) {
            glm::vec3 originalPos = m_originalVertices->vertex(i);
            m_noiseCache[i] = generateNoise(originalPos, i);
        }
        m_cacheValid = true;
    }
    
    // Apply deformation
    for (GLuint i = 0; i < numVertices; ++i) {
        glm::vec3 originalPos = m_originalVertices->vertex(i);
        glm::vec3 noise = m_noiseCache[i] * m_weight;
        glm::vec3 deformedPos;
        
        switch (m_mode) {
            case DeformationMode::ABSOLUTE:
                deformedPos = noise;
                break;
                
            case DeformationMode::ADDITIVE:
                deformedPos = originalPos + noise;
                break;
                
            case DeformationMode::DIRECTIONAL: {
                float noiseLength = glm::length(noise);
                deformedPos = originalPos + m_direction * noiseLength;
                break;
            }
            
            case DeformationMode::RADIAL: {
                glm::vec3 toCenter = originalPos - m_center;
                float distance = glm::length(toCenter);
                if (distance > 0.001f) {
                    glm::vec3 radialDir = toCenter / distance;
                    float noiseLength = glm::length(noise);
                    deformedPos = originalPos + radialDir * noiseLength;
                } else {
                    deformedPos = originalPos;
                }
                break;
            }
            
            case DeformationMode::SURFACE_NORMAL:
                // For surface normal mode, we'd need access to normals
                // For now, use additive mode as fallback
                deformedPos = originalPos + noise;
                break;
                
            default:
                deformedPos = originalPos + noise;
                break;
        }
        
        m_deformedVertices->setVertex(i, deformedPos.x, deformedPos.y, deformedPos.z);
    }
}

std::unique_ptr<Deformer> RandomDeformer::clone() const {
    auto cloned = std::make_unique<RandomDeformer>(m_noiseType, m_mode);
    cloned->setIntensity(m_intensity);
    cloned->setFrequency(m_frequency);
    cloned->setOffset(m_offset);
    cloned->setDirection(m_direction);
    cloned->setCenter(m_center);
    cloned->setTime(m_time);
    cloned->setSeed(m_seed);
    cloned->setOctaves(m_octaves);
    cloned->setPersistence(m_persistence);
    cloned->setLacunarity(m_lacunarity);
    cloned->setWeight(m_weight);
    cloned->setEnabled(m_enabled);
    return cloned;
}

glm::vec3 RandomDeformer::generateNoise(const glm::vec3& position, GLuint vertexIndex) const {
    glm::vec3 samplePos = (position + m_offset) * m_frequency;
    glm::vec3 noise;
    
    switch (m_noiseType) {
        case NoiseType::UNIFORM:
            // Use vertex index to ensure consistency
            m_rng.seed(m_seed + vertexIndex);
            noise = glm::vec3(
                m_uniformDist(m_rng),
                m_uniformDist(m_rng),
                m_uniformDist(m_rng)
            ) * m_intensity;
            break;
            
        case NoiseType::GAUSSIAN:
            m_rng.seed(m_seed + vertexIndex);
            noise = glm::vec3(
                m_gaussianDist(m_rng),
                m_gaussianDist(m_rng),
                m_gaussianDist(m_rng)
            ) * m_intensity;
            break;
            
        case NoiseType::PERLIN: {
            float noiseValue = perlinNoise(samplePos + glm::vec3(m_time * 0.1f));
            noise = glm::vec3(noiseValue) * m_intensity;
            break;
        }
        
        case NoiseType::SIMPLEX: {
            float noiseValue = simplexNoise(samplePos + glm::vec3(m_time * 0.1f));
            noise = glm::vec3(noiseValue) * m_intensity;
            break;
        }
        
        case NoiseType::CELLULAR: {
            float noiseValue = cellularNoise(samplePos);
            noise = glm::vec3(noiseValue) * m_intensity;
            break;
        }
        
        case NoiseType::FRACTAL: {
            float noiseValue = fractalNoise(samplePos + glm::vec3(m_time * 0.1f));
            noise = glm::vec3(noiseValue) * m_intensity;
            break;
        }
    }
    
    return noise;
}

// Simplified Perlin noise implementation
float RandomDeformer::perlinNoise(const glm::vec3& position) const {
    // This is a simplified version - you might want to use a proper noise library
    int xi = (int)std::floor(position.x) & 255;
    int yi = (int)std::floor(position.y) & 255;
    int zi = (int)std::floor(position.z) & 255;
    
    float xf = position.x - std::floor(position.x);
    float yf = position.y - std::floor(position.y);
    float zf = position.z - std::floor(position.z);
    
    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);
    
    // Hash function (simplified)
    auto hash = [this](int x, int y, int z) -> int {
        return ((x * 73856093) ^ (y * 19349663) ^ (z * 83492791) ^ m_seed) & 255;
    };
    
    // Sample gradients at cube corners
    int aaa = hash(xi, yi, zi);
    int aba = hash(xi, yi + 1, zi);
    int aab = hash(xi, yi, zi + 1);
    int abb = hash(xi, yi + 1, zi + 1);
    int baa = hash(xi + 1, yi, zi);
    int bba = hash(xi + 1, yi + 1, zi);
    int bab = hash(xi + 1, yi, zi + 1);
    int bbb = hash(xi + 1, yi + 1, zi + 1);
    
    // Interpolate
    float x1 = lerp(u, grad(aaa, xf, yf, zf), grad(baa, xf - 1, yf, zf));
    float x2 = lerp(u, grad(aba, xf, yf - 1, zf), grad(bba, xf - 1, yf - 1, zf));
    float y1 = lerp(v, x1, x2);
    
    x1 = lerp(u, grad(aab, xf, yf, zf - 1), grad(bab, xf - 1, yf, zf - 1));
    x2 = lerp(u, grad(abb, xf, yf - 1, zf - 1), grad(bbb, xf - 1, yf - 1, zf - 1));
    float y2 = lerp(v, x1, x2);
    
    return lerp(w, y1, y2);
}

float RandomDeformer::simplexNoise(const glm::vec3& position) const {
    // Simplified 3D simplex noise - for production use a proper noise library
    const float F3 = 1.0f / 3.0f;
    const float G3 = 1.0f / 6.0f;
    
    float s = (position.x + position.y + position.z) * F3;
    int i = (int)std::floor(position.x + s);
    int j = (int)std::floor(position.y + s);
    int k = (int)std::floor(position.z + s);
    
    float t = (i + j + k) * G3;
    float x0 = position.x - (i - t);
    float y0 = position.y - (j - t);
    float z0 = position.z - (k - t);
    
    // For simplicity, return a basic noise value
    return std::sin(position.x * 12.9898f + position.y * 78.233f + position.z * 37.719f) * 0.5f + 0.5f;
}

float RandomDeformer::cellularNoise(const glm::vec3& position) const {
    // Simplified cellular noise
    int xi = (int)std::floor(position.x);
    int yi = (int)std::floor(position.y);
    int zi = (int)std::floor(position.z);
    
    float minDist = 10000.0f;
    
    // Check surrounding cells
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                int cellX = xi + dx;
                int cellY = yi + dy;
                int cellZ = zi + dz;
                
                // Generate random point in cell
                unsigned int hash = ((cellX * 73856093) ^ (cellY * 19349663) ^ 
                                   (cellZ * 83492791) ^ m_seed) & 0x7fffffff;
                float fx = (hash % 1000) / 1000.0f;
                hash = (hash * 16807) & 0x7fffffff;
                float fy = (hash % 1000) / 1000.0f;
                hash = (hash * 16807) & 0x7fffffff;
                float fz = (hash % 1000) / 1000.0f;
                
                glm::vec3 cellPoint(cellX + fx, cellY + fy, cellZ + fz);
                float dist = glm::length(position - cellPoint);
                minDist = std::min(minDist, dist);
            }
        }
    }
    
    return minDist;
}

float RandomDeformer::fractalNoise(const glm::vec3& position) const {
    float value = 0.0f;
    float amplitude = 1.0f;
    glm::vec3 frequency(1.0f);
    
    for (int i = 0; i < m_octaves; ++i) {
        value += perlinNoise(position * frequency) * amplitude;
        amplitude *= m_persistence;
        frequency *= m_lacunarity;
    }
    
    return value;
}

float RandomDeformer::fade(float t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float RandomDeformer::lerp(float t, float a, float b) const {
    return a + t * (b - a);
}

float RandomDeformer::grad(int hash, float x, float y, float z) const {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
