#pragma once

#include <ivf/mesh_node.h>
#include <ivf/composite_node.h>
#include <assimp/scene.h>
#include <string>

namespace ivf {

class ModelLoader {
private:
    static void processNode(const aiScene *scene, aiNode *node, std::shared_ptr<CompositeNode> compNode,
                            aiMatrix4x4 parentTransform);
    static void processAiMesh(const aiScene *scene, aiMesh *aiMesh, std::shared_ptr<MeshNode> meshNode);
    static void processMaterial(aiMaterial *aiMat, std::shared_ptr<Mesh> mesh);
    static void processMaterial(aiMaterial *aiMat, std::shared_ptr<MeshNode> node);

public:
    // Prevent instantiation since this is a utility class
    ModelLoader() = delete;
    ~ModelLoader() = delete;

    // Main model loading function
    static std::shared_ptr<CompositeNode> loadModel(const std::string &path);
};

} // namespace ivf
