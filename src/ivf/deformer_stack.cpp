#include <ivf/deformer_stack.h>
#include <ivf/utils.h>

using namespace ivf;

DeformerStack::DeformerStack() {}

std::shared_ptr<DeformerStack> DeformerStack::create() {
    return std::make_shared<DeformerStack>();
}

void DeformerStack::addDeformer(std::shared_ptr<Deformer> deformer) {
    m_deformers.push_back(deformer);
}

void DeformerStack::removeDeformer(std::shared_ptr<Deformer> deformer) {
    auto it = std::find(m_deformers.begin(), m_deformers.end(), deformer);
    if (it != m_deformers.end()) {
        m_deformers.erase(it);
    }
}

void DeformerStack::removeDeformer(size_t index) {
    if (index < m_deformers.size()) {
        m_deformers.erase(m_deformers.begin() + index);
    }
}

void DeformerStack::clear() {
    m_deformers.clear();
}

void DeformerStack::setInput(std::shared_ptr<Vertices> vertices) {
    m_inputVertices = vertices;
    if (vertices) {
        m_outputVertices = std::make_shared<Vertices>(vertices->rows());
    }
}

std::shared_ptr<Vertices> DeformerStack::getOutput() {
    return m_outputVertices;
}

void DeformerStack::apply() {
    if (!m_inputVertices || !m_outputVertices || m_deformers.empty()) return;
    
    // Start with input vertices
    auto currentVertices = m_inputVertices;
    
    // Apply each deformer in sequence
    for (auto& deformer : m_deformers) {
        if (deformer->enabled()) {
            deformer->setInput(currentVertices);
            deformer->apply();
            currentVertices = deformer->getOutput();
        }
    }
    
    // Copy final result to output
    if (currentVertices) {
        std::memcpy(m_outputVertices->data(), currentVertices->data(), 
                   currentVertices->memSize());
    }
}

void DeformerStack::reset() {
    for (auto& deformer : m_deformers) {
        deformer->reset();
    }
}

size_t DeformerStack::count() const {
    return m_deformers.size();
}

std::shared_ptr<Deformer> DeformerStack::deformer(size_t index) {
    if (index < m_deformers.size()) {
        return m_deformers[index];
    }
    return nullptr;
}

void DeformerStack::setEnabled(bool enabled) {
    for (auto& deformer : m_deformers) {
        deformer->setEnabled(enabled);
    }
}

void DeformerStack::setWeight(float weight) {
    for (auto& deformer : m_deformers) {
        deformer->setWeight(weight);
    }
}