#pragma once

#include <ivf/deformer.h>

namespace ivf {

class DeformerStack : public Base {
private:
    std::vector<std::shared_ptr<Deformer>> m_deformers;
    std::shared_ptr<Vertices> m_inputVertices;
    std::shared_ptr<Vertices> m_outputVertices;
    
public:
    DeformerStack();
    
    static std::shared_ptr<DeformerStack> create();
    
    void addDeformer(std::shared_ptr<Deformer> deformer);
    void removeDeformer(std::shared_ptr<Deformer> deformer);
    void removeDeformer(size_t index);
    void clear();
    
    void setInput(std::shared_ptr<Vertices> vertices);
    std::shared_ptr<Vertices> getOutput();
    
    void apply(); // Apply all deformers in sequence
    void reset(); // Reset all deformers
    
    size_t count() const;
    std::shared_ptr<Deformer> deformer(size_t index);
    
    // Animation support
    void setEnabled(bool enabled);
    void setWeight(float weight);
};

};