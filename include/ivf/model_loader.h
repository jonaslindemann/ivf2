#pragma once

#include <ivf/mesh_node.h>
#include <assimp/scene.h>
#include <string>

namespace ivf {

class ModelLoader {
private:
    static void processNode(const aiScene *scene, aiNode *node, std::shared_ptr<MeshNode> meshNode);

    static void processAiMesh(const aiScene *scene, aiMesh *aiMesh, std::shared_ptr<MeshNode> meshNode);

public:
    // Prevent instantiation since this is a utility class
    ModelLoader() = delete;
    ~ModelLoader() = delete;

    // Main model loading function
    static std::shared_ptr<MeshNode> loadModel(const std::string &path);
};

} // namespace ivf
