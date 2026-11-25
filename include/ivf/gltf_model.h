#pragma once

/**
 * @file gltf_model.h
 * @brief glTF 2.0 models: hierarchy, PBR materials, skinning, morph targets and animations.
 */

#include <ivf/composite_node.h>
#include <ivf/mesh_node.h>
#include <ivf/pbr_material.h>
#include <ivf/result.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ivf {

class GltfModel;
using GltfModelPtr = std::shared_ptr<GltfModel>;

/**
 * @class GltfModel
 * @brief A glTF 2.0 model (.gltf with its files, or binary .glb) as a scene graph node.
 *
 * Every glTF node becomes a CompositeNode with the node's translation, rotation and scale
 * (child of its parent node); a node's mesh becomes a MeshNode child with one Mesh per
 * primitive, each with its own PBRMaterial (base colour, metallic-roughness, normal,
 * occlusion and emissive textures, alpha modes, vertex colours).
 *
 * Animations move nodes (translation, rotation, scale, with step, linear or cubic spline
 * interpolation), morph-target weights, and skinned meshes through their joints. Skinning
 * and morph targets are applied on the CPU to the mesh vertices, so animated models work
 * with every shader, in shadow maps and with selection.
 *
 * @code
 * auto model = ivf::GltfModel::load("assets/models/robot.glb"); // in onSetup(): textures need OpenGL
 * if (!model)
 *     return -1; // the reason is logged
 * add(*model);
 *
 * (*model)->setAnimation(0);
 * // every frame
 * (*model)->setTime(float(elapsedTime()));
 * @endcode
 *
 * Not supported (yet): cameras and lights in the file (KHR_lights_punctual), texture
 * transforms, Draco/meshopt compression and other required extensions (the file is
 * refused), points and lines.
 */
class GltfModel : public CompositeNode {
public:
    GltfModel();
    ~GltfModel() override;

    [[nodiscard]] static GltfModelPtr create();

    /**
     * @brief Load a .gltf or .glb file.
     *
     * Images are decoded and uploaded as textures, so call it with a current OpenGL
     * context (in onSetup()), unless the model has no textures.
     *
     * @return The model, or an Error (FileNotFound, UnsupportedFormat, InvalidData).
     */
    [[nodiscard]] static Result<GltfModelPtr> load(const std::string &path);

    /**
     * @brief Load the file again and replace the model's contents in place (the node stays
     *        in the scene). Keeps the current animation (if the file still has it), time and
     *        looping, and the model's own transform; materials are new.
     * @param path File to load; empty: the file the model was loaded from.
     * @return Nothing, or an Error (the model is unchanged then).
     */
    Result<void> reload(const std::string &path = {});

    /** @brief File the model was loaded from. */
    [[nodiscard]] const std::string &path() const;

    // ---- Nodes ----------------------------------------------------------------

    /** @brief Number of glTF nodes. */
    [[nodiscard]] int nodeCount() const;

    /** @brief The scene graph node of glTF node @p index (nullptr if out of range). */
    [[nodiscard]] CompositeNodePtr node(int index) const;

    /** @brief The node named @p name (the first one), nullptr if there is none. */
    [[nodiscard]] CompositeNodePtr findNode(std::string_view name) const;

    /** @brief Index of the node named @p name, -1 if there is none. */
    [[nodiscard]] int findNodeIndex(std::string_view name) const;

    /** @brief The MeshNodes of the model (one per glTF node with a mesh). */
    [[nodiscard]] std::vector<MeshNodePtr> meshNodes() const;

    /**
     * @brief The model's materials (one per glTF material, shared by the primitives using it).
     *
     * For example to light them with an EnvironmentMap or to switch off their own tone
     * mapping when the window tone maps:
     * @code
     * for (auto &material : model->materials())
     * {
     *     material->setEnvironment(environment);
     *     material->setToneMapping(false);
     * }
     * @endcode
     */
    [[nodiscard]] std::vector<PBRMaterialPtr> materials() const;

    // ---- Animation ------------------------------------------------------------

    /** @brief Number of animations in the file. */
    [[nodiscard]] int animationCount() const;

    /** @brief Name of animation @p index ("" if unnamed or out of range). */
    [[nodiscard]] std::string animationName(int index) const;

    /** @brief Length of animation @p index in seconds. */
    [[nodiscard]] float animationDuration(int index) const;

    /** @brief Index of the animation named @p name, -1 if there is none. */
    [[nodiscard]] int findAnimation(std::string_view name) const;

    /**
     * @brief Choose the animation setTime() plays (-1: none, the model keeps its pose).
     *
     * Choosing one resets the nodes to their rest pose first, so channels of the previous
     * animation do not linger.
     */
    void setAnimation(int index);

    /** @brief The animation setTime() plays, -1 if none. */
    [[nodiscard]] int animation() const;

    /** @brief Loop the animation (default) or hold its last pose after the end. */
    void setLooping(bool flag);
    [[nodiscard]] bool looping() const;

    /**
     * @brief Pose the model at @p seconds into the current animation and update skinned
     *        and morphed meshes.
     */
    void setTime(float seconds);

    /** @brief The time of the last setTime(). */
    [[nodiscard]] float time() const;

    /**
     * @brief Set the morph-target weights of node @p index (for meshes with morph targets).
     *
     * Applied by the next setTime() or update(); the current animation overrides weights
     * it animates.
     */
    void setMorphWeights(int index, const std::vector<float> &weights);

    /** @brief Morph-target weights of node @p index (empty if its mesh has no targets). */
    [[nodiscard]] std::vector<float> morphWeights(int index) const;

    /**
     * @brief Recompute skinned and morphed meshes from the current node transforms and
     *        weights, after moving joints by hand (node(i)->setPos(), setRotation(),
     *        setScale()) or setting weights. The next setTime() poses the animated nodes again.
     */
    void update();

    /** @brief True if any mesh is skinned or has morph targets (setTime() changes vertices). */
    [[nodiscard]] bool hasDeformation() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    friend struct GltfLoaderAccess;
};

} // namespace ivf
