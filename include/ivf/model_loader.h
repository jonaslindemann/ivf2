#pragma once
/**
 * @file model_loader.h
 * @brief Loading 3D model files into the scene graph.
 *
 * Model loading uses assimp, which is a private dependency of ivf. Check
 * IVF2_HAS_ASSIMP (ivf/config.h) to see whether this build supports it.
 */

#include <ivf/composite_node.h>
#include <ivf/mesh_node.h>

#include <memory>
#include <string>

namespace ivf {

/**
 * @class ModelLoader
 * @brief Loads model files (OBJ, FBX, glTF, 3DS, ... - everything assimp reads).
 */
class ModelLoader {
public:
    ModelLoader() = delete;
    ~ModelLoader() = delete;

    /**
     * @brief Load a model file as a node hierarchy.
     * @param path Path to the model file.
     * @return The root of the loaded hierarchy.
     * @throws std::runtime_error if the file cannot be loaded, or if ivf2 was built
     *         without assimp (IVF2_WITH_ASSIMP=OFF).
     */
    static std::shared_ptr<CompositeNode> loadModel(const std::string &path);
};

} // namespace ivf
