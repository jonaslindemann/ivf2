#pragma once
/**
 * @file model_loader.h
 * @brief Loading 3D model files into the scene graph.
 *
 * glTF files (.gltf, .glb) are read by GltfModel; everything else uses assimp, which is a
 * private dependency of ivf. Check IVF2_HAS_ASSIMP (ivf/config.h) to see whether this
 * build supports the other formats.
 */

#include <ivf/composite_node.h>
#include <ivf/mesh_node.h>
#include <ivf/result.h>

#include <memory>
#include <string>

namespace ivf {

/**
 * @class ModelLoader
 * @brief Loads model files: glTF through GltfModel (also without assimp), and OBJ, FBX,
 *        3DS, ... (everything assimp reads).
 */
class ModelLoader {
public:
    ModelLoader() = delete;
    ~ModelLoader() = delete;

    /**
     * @brief Load a model file as a node hierarchy.
     * @param path Path to the model file.
     * @return The root of the loaded hierarchy (a GltfModel for .gltf/.glb files), or an
     *         Error (FileNotFound, UnsupportedFormat, NotSupported for other formats when
     *         built without assimp). Errors are also logged.
     */
    [[nodiscard]] static Result<std::shared_ptr<CompositeNode>> load(const std::string &path);

    /**
     * @brief Load a model file; throws on failure.
     * @deprecated Use load(), which returns a Result instead of throwing.
     * @throws std::runtime_error if the file cannot be loaded.
     */
    [[deprecated("use ModelLoader::load(), which returns a Result")]]
    static std::shared_ptr<CompositeNode> loadModel(const std::string &path);
};

} // namespace ivf
