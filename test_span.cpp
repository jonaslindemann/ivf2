// Test file to verify std::span compiles
#include <span>
#include <vector>
#include <memory>

struct TestTexture {};

int main() {
    std::vector<std::shared_ptr<TestTexture>> textures;
    textures.push_back(std::make_shared<TestTexture>());
    
    std::span<const std::shared_ptr<TestTexture>> span_view(textures);
    
    for (const auto& tex : span_view) {
        // Use tex
    }
    
    return 0;
}
