#pragma once

/**
 * @file scene_serializer.h
 * @brief JSON-based scene graph save/load using nlohmann-json.
 *
 * Saves and loads the scene graph rooted at a CompositeNode.  Node-specific
 * geometry parameters (e.g., Sphere radius, Box size) are round-tripped via
 * the PropertyInspectable interface, so any node that registers its geometry
 * parameters as properties can be fully serialised without special-casing.
 *
 * Usage:
 * @code
 * // Register built-in types once at startup (call from onSetup or main):
 * SceneSerializer::registerBuiltinTypes();
 *
 * // Save
 * SceneSerializer::save(scene->sceneRoot(), "my_scene.json");
 *
 * // Load (returns a new CompositeNode; add it to your scene)
 * auto loaded = SceneSerializer::load("my_scene.json");
 * if (loaded) scene->add(loaded);
 * @endcode
 */

#include <ivf/node.h>
#include <ivf/composite_node.h>
#include <ivf/transform_node.h>
#include <ivf/material.h>

#include <nlohmann/json.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace ivf {

/**
 * @class SceneSerializer
 * @brief Stateless utility class for JSON scene serialisation/deserialisation.
 */
class SceneSerializer {
public:
    SceneSerializer() = delete;

    // ---- Save / load -------------------------------------------------------

    /**
     * @brief Save the scene graph rooted at @p root to a JSON file.
     * @param root Root composite node.
     * @param path File path to write.
     * @return true on success.
     */
    static bool save(CompositeNodePtr root, std::string_view path);

    /**
     * @brief Load a scene graph from a JSON file.
     * @param path File path to read.
     * @return Loaded root CompositeNode, or nullptr on failure.
     */
    static CompositeNodePtr load(std::string_view path);

    // ---- Type registry -----------------------------------------------------

    /**
     * @brief Register a node type for deserialisation.
     * @param typeName  The string stored in the "type" JSON field.
     * @param factory   Zero-argument factory returning a NodePtr.
     *
     * Example:
     * @code
     * SceneSerializer::registerType("Sphere", []{ return Sphere::create(); });
     * @endcode
     */
    static void registerType(std::string_view typeName, std::function<NodePtr()> factory);

    /**
     * @brief Register all node types declared in ivf/nodes.h.
     * Call once at application startup before saving or loading scenes.
     */
    static void registerBuiltinTypes();

private:
    static nlohmann::json    serializeNode(NodePtr node);
    static NodePtr           deserializeNode(const nlohmann::json& j);

    static void serializeTransform(TransformNodePtr tn, nlohmann::json& j);
    static void deserializeTransform(TransformNodePtr tn, const nlohmann::json& j);

    static void serializeMaterial(MaterialPtr mat, nlohmann::json& j);
    static void deserializeMaterial(MaterialPtr mat, const nlohmann::json& j);

    static void serializeProperties(NodePtr node, nlohmann::json& j);
    static void deserializeProperties(NodePtr node, const nlohmann::json& j);

    static std::map<std::string, std::function<NodePtr()>>& typeRegistry();
};

} // namespace ivf
