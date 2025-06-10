#pragma once

#include <ivf/base.h>
#include <ivf/vertices.h>
#include <ivf/mesh.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <functional>

namespace ivf {

// Forward declarations
class Mesh;
class MeshNode;

// Base deformer class
class Deformer : public Base {
protected:
    std::shared_ptr<Vertices> m_originalVertices;
    std::shared_ptr<Vertices> m_deformedVertices;
    bool m_enabled;
    float m_weight; // Blend weight for combining deformers
    
public:
    Deformer();
    virtual ~Deformer() = default;
    
    // Core deformation interface
    virtual void setInput(std::shared_ptr<Vertices> vertices);
    virtual std::shared_ptr<Vertices> getOutput();
    virtual void apply() = 0;
    virtual void reset();
    
    // Animation support
    void setEnabled(bool enabled);
    bool enabled() const;
    void setWeight(float weight);
    float weight() const;
    
    // Create a copy for animation keyframes
    virtual std::unique_ptr<Deformer> clone() const = 0;
};

};