#pragma once

#include <ivf/deformer.h>
#include <glm/glm.hpp>
#include <functional>
#include <random>
#include <vector>
#include <memory>

namespace ivf {

// Function-based deformer that applies a displacement function f(x, y, z) -> vec3
class FunctionDeformer : public Deformer {
public:
    // Function signature: takes position, returns displacement
    using DisplacementFunction = std::function<glm::vec3(const glm::vec3&)>;
    
private:
    DisplacementFunction m_displacementFunction;
    glm::vec3 m_scale;              // Scale factor for displacement
    glm::vec3 m_offset;             // Offset added to input coordinates
    bool m_useLocalSpace;           // Whether to apply function in local or world space
    float m_time;                   // Time parameter for animated functions
    
public:
    FunctionDeformer();
    FunctionDeformer(const DisplacementFunction& func);
    
    static std::shared_ptr<FunctionDeformer> create();
    static std::shared_ptr<FunctionDeformer> create(const DisplacementFunction& func);
    
    // Set the displacement function
    void setFunction(const DisplacementFunction& func);
    
    // Scale the displacement output
    void setScale(const glm::vec3& scale);
    void setScale(float uniformScale);
    glm::vec3 scale() const;
    
    // Offset the input coordinates
    void setOffset(const glm::vec3& offset);
    glm::vec3 offset() const;
    
    // Local vs world space transformation
    void setUseLocalSpace(bool useLocal);
    bool useLocalSpace() const;
    
    // Time parameter for animated functions
    void setTime(float time);
    float time() const;
    
    virtual void apply() override;
    virtual std::unique_ptr<Deformer> clone() const override;
    
    // Predefined common displacement functions
    static DisplacementFunction createSineWave(float frequency = 1.0f, float amplitude = 1.0f, 
                                             const glm::vec3& direction = glm::vec3(0, 1, 0));
    
    static DisplacementFunction createRipple(float frequency = 1.0f, float amplitude = 1.0f,
                                           const glm::vec3& center = glm::vec3(0, 0, 0));
    
    static DisplacementFunction createTurbulence(float scale = 1.0f, float intensity = 1.0f,
                                               int octaves = 4);
    
    static DisplacementFunction createSpiral(float pitch = 1.0f, float radius = 1.0f);
    
    static DisplacementFunction createTwirl(const glm::vec3& center = glm::vec3(0, 0, 0),
                                          float strength = 1.0f, float falloff = 1.0f);
    
    static DisplacementFunction createGravityWell(const glm::vec3& center = glm::vec3(0, 0, 0),
                                                float strength = 1.0f, float radius = 5.0f);
};

typedef std::shared_ptr<FunctionDeformer> FunctionDeformerPtr;

};