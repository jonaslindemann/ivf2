#include <ivf/scene_serializer.h>

#include <ivf/nodes.h>
#include <ivf/logger.h>
#include <filesystem>

#include <nlohmann/json.hpp>

#include <fstream>
#include <typeindex>
#include <unordered_map>

using namespace ivf;
using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Internal registries
// ---------------------------------------------------------------------------
struct Registries {
    std::map<std::string, std::function<NodePtr()>> factories;
    std::unordered_map<std::type_index, std::string> typeNames;
};

static Registries& reg()
{
    static Registries r;
    return r;
}

// ---------------------------------------------------------------------------
void SceneSerializer::registerType(std::string_view typeName, std::function<NodePtr()> factory)
{
    std::string name(typeName);
    // Create one dummy instance to capture its type_index for serialisation.
    auto instance = factory();
    const auto &instanceRef = *instance;
    reg().typeNames[std::type_index(typeid(instanceRef))] = name;
    reg().factories[name] = std::move(factory);
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
void SceneSerializer::registerBuiltinTypes()
{
    registerType("CompositeNode",    []{ return CompositeNode::create(); });
    registerType("Sphere",           []{ return Sphere::create(); });
    registerType("Box",              []{ return Box::create(); });
    registerType("Cube",             []{ return Cube::create(); });
    registerType("Cylinder",         []{ return Cylinder::create(); });
    registerType("CappedCylinder",   []{ return CappedCylinder::create(); });
    registerType("Cone",             []{ return Cone::create(); });
    registerType("CappedCone",       []{ return CappedCone::create(); });
    registerType("Plane",            []{ return Plane::create(); });
    registerType("Disk",             []{ return Disk::create(); });
    registerType("Capsule",          []{ return Capsule::create(); });
    registerType("RoundedBox",       []{ return RoundedBox::create(); });
    registerType("Tube",             []{ return Tube::create(); });
    registerType("CappedTube",       []{ return CappedTube::create(); });
    registerType("Dodecahedron",     []{ return Dodecahedron::create(); });
}

// ---------------------------------------------------------------------------
// Helpers: vec3 / vec4 ↔ json
// ---------------------------------------------------------------------------
// nlohmann-json is private to this file: the helpers below are internal.
namespace {

static json vec3ToJson(const glm::vec3& v) { return json::array({v.x, v.y, v.z}); }
static json vec4ToJson(const glm::vec4& v) { return json::array({v.x, v.y, v.z, v.w}); }

static glm::vec3 jsonToVec3(const json& j, glm::vec3 def = {0,0,0})
{
    if (!j.is_array() || j.size() < 3) return def;
    return { j[0].get<float>(), j[1].get<float>(), j[2].get<float>() };
}

static glm::vec4 jsonToVec4(const json& j, glm::vec4 def = {1,1,1,1})
{
    if (!j.is_array() || j.size() < 4) return def;
    return { j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>() };
}

// ---------------------------------------------------------------------------
void serializeTransform(TransformNodePtr tn, json& j)
{
    if (!tn) return;
    j["transform"]["pos"]        = vec3ToJson(tn->pos());
    j["transform"]["eulerAngles"]= vec3ToJson(tn->eulerAngles());
    j["transform"]["scale"]      = vec3ToJson(tn->scale());
    j["transform"]["rotAxis"]    = vec3ToJson(tn->rotAxis());
    j["transform"]["rotAngle"]   = tn->rotAngle();
}

void deserializeTransform(TransformNodePtr tn, const json& j)
{
    if (!tn || !j.contains("transform")) return;
    const auto& t = j["transform"];
    if (t.contains("pos"))         tn->setPos(jsonToVec3(t["pos"]));
    if (t.contains("eulerAngles")) tn->setEulerAngles(jsonToVec3(t["eulerAngles"]));
    if (t.contains("scale"))       tn->setScale(jsonToVec3(t["scale"], {1,1,1}));
    if (t.contains("rotAxis"))     tn->setRotAxis(jsonToVec3(t["rotAxis"], {0,1,0}));
    if (t.contains("rotAngle"))    tn->setRotAngle(t["rotAngle"].get<float>());
}

// ---------------------------------------------------------------------------
void serializeMaterial(MaterialPtr mat, json& j)
{
    if (!mat) return;
    j["material"]["diffuse"]     = vec4ToJson(mat->diffuseColor());
    j["material"]["specular"]    = vec4ToJson(mat->specularColor());
    j["material"]["ambient"]     = vec4ToJson(mat->ambientColor());
    j["material"]["shininess"]   = mat->shininess();
    j["material"]["alpha"]       = mat->alpha();
    j["material"]["useLighting"] = mat->useLighting();
}

void deserializeMaterial(MaterialPtr mat, const json& j)
{
    if (!mat || !j.contains("material")) return;
    const auto& m = j["material"];
    if (m.contains("diffuse"))     mat->setDiffuseColor(jsonToVec4(m["diffuse"]));
    if (m.contains("specular"))    mat->setSpecularColor(jsonToVec4(m["specular"]));
    if (m.contains("ambient"))     mat->setAmbientColor(jsonToVec4(m["ambient"]));
    if (m.contains("shininess"))   mat->setShininess(m["shininess"].get<float>());
    if (m.contains("alpha"))       mat->setAlpha(m["alpha"].get<float>());
    if (m.contains("useLighting")) mat->setUseLighting(m["useLighting"].get<bool>());
}

// ---------------------------------------------------------------------------
void serializeProperties(NodePtr node, json& j)
{
    if (!node) return;
    const auto& props = node->properties();
    json props_j;
    for (const auto& prop : props) {
        if (prop.readOnly) continue;
        std::visit([&](auto* ptr) {
            using T = std::remove_pointer_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, double>)
                props_j[prop.name] = *ptr;
            else if constexpr (std::is_same_v<T, float>)
                props_j[prop.name] = *ptr;
            else if constexpr (std::is_same_v<T, int>)
                props_j[prop.name] = *ptr;
            else if constexpr (std::is_same_v<T, bool>)
                props_j[prop.name] = *ptr;
            else if constexpr (std::is_same_v<T, std::string>)
                props_j[prop.name] = *ptr;
            else if constexpr (std::is_same_v<T, glm::vec3>)
                props_j[prop.name] = vec3ToJson(*ptr);
            else if constexpr (std::is_same_v<T, glm::vec4>)
                props_j[prop.name] = vec4ToJson(*ptr);
            // Other types (uvec3, uvec4, etc.) skipped
        }, prop.value);
    }
    if (!props_j.is_null()) j["properties"] = props_j;
}

void deserializeProperties(NodePtr node, const json& j)
{
    if (!node || !j.contains("properties")) return;
    const auto& props_j = j["properties"];
    const auto& props = node->properties();
    for (const auto& prop : props) {
        if (prop.readOnly || !props_j.contains(prop.name)) continue;
        const auto& val = props_j[prop.name];
        std::visit([&](auto* ptr) {
            using T = std::remove_pointer_t<decltype(ptr)>;
            try {
                if constexpr (std::is_same_v<T, double>)
                    *ptr = val.get<double>();
                else if constexpr (std::is_same_v<T, float>)
                    *ptr = val.get<float>();
                else if constexpr (std::is_same_v<T, int>)
                    *ptr = val.get<int>();
                else if constexpr (std::is_same_v<T, bool>)
                    *ptr = val.get<bool>();
                else if constexpr (std::is_same_v<T, std::string>)
                    *ptr = val.get<std::string>();
                else if constexpr (std::is_same_v<T, glm::vec3>)
                    *ptr = jsonToVec3(val);
                else if constexpr (std::is_same_v<T, glm::vec4>)
                    *ptr = jsonToVec4(val);
            } catch (...) { /* ignore type mismatch */ }
        }, prop.value);
    }
    node->notifyPropertyChanged(""); // trigger geometry rebuild
}

// ---------------------------------------------------------------------------
json serializeNode(NodePtr node)
{
    json j;

    // Type name
    const auto &nodeRef = *node;
    auto it = reg().typeNames.find(std::type_index(typeid(nodeRef)));
    j["type"] = (it != reg().typeNames.end()) ? it->second : "Unknown";

    // Base properties
    j["name"]    = node->name();
    j["visible"] = node->visible();

    // Transform
    if (auto tn = std::dynamic_pointer_cast<TransformNode>(node))
        serializeTransform(tn, j);

    // Material
    if (node->material())
        serializeMaterial(node->material(), j);

    // Node-specific geometry parameters via PropertyInspectable
    serializeProperties(node, j);

    // Children (CompositeNode)
    if (auto cn = std::dynamic_pointer_cast<CompositeNode>(node)) {
        json children = json::array();
        for (auto& child : cn->nodes())
            children.push_back(serializeNode(child));
        j["children"] = children;
    }

    return j;
}

// ---------------------------------------------------------------------------
NodePtr deserializeNode(const json& j)
{
    if (!j.contains("type")) return nullptr;
    std::string typeName = j["type"].get<std::string>();

    auto it = reg().factories.find(typeName);
    if (it == reg().factories.end()) {
        logWarning("SceneSerializer: unknown type '" + typeName + "' — skipped", "SceneSerializer");
        return nullptr;
    }

    NodePtr node = it->second();
    if (!node) return nullptr;

    // Base properties
    if (j.contains("name"))    node->setName(j["name"].get<std::string>());
    if (j.contains("visible")) node->setVisible(j["visible"].get<bool>());

    // Transform
    if (auto tn = std::dynamic_pointer_cast<TransformNode>(node))
        deserializeTransform(tn, j);

    // Material
    if (j.contains("material")) {
        auto mat = Material::create();
        deserializeMaterial(mat, j);
        node->setMaterial(mat);
    }

    // Node-specific properties
    deserializeProperties(node, j);

    // Children
    if (j.contains("children") && j["children"].is_array()) {
        if (auto cn = std::dynamic_pointer_cast<CompositeNode>(node)) {
            for (const auto& childJ : j["children"]) {
                auto child = deserializeNode(childJ);
                if (child) cn->add(child);
            }
        }
    }

    return node;
}

} // namespace

// ---------------------------------------------------------------------------
Result<void> SceneSerializer::saveFile(CompositeNodePtr root, std::string_view path)
{
    const std::string savePath(path);
    if (!root)
        return makeError(ErrorCode::InvalidArgument, "Cannot save scene to '" + savePath + "': no root node",
                         "SceneSerializer");

    json j = serializeNode(root);
    std::ofstream f{savePath};
    if (!f.is_open())
        return makeError(ErrorCode::WriteFailed, "Cannot open '" + savePath + "' for writing", "SceneSerializer");
    f << j.dump(2);
    if (!f.good())
        return makeError(ErrorCode::WriteFailed, "Could not write scene to '" + savePath + "'", "SceneSerializer");
    return {};
}

Result<CompositeNodePtr> SceneSerializer::loadFile(std::string_view path)
{
    const std::string pathStr(path);
    if (!std::filesystem::exists(std::filesystem::path(pathStr)))
        return makeError(ErrorCode::FileNotFound, "Could not load scene '" + pathStr + "': file not found",
                         "SceneSerializer");

    std::ifstream f{pathStr};
    if (!f.is_open())
        return makeError(ErrorCode::ReadFailed, "Cannot open '" + pathStr + "' for reading", "SceneSerializer");

    json j;
    try
    {
        f >> j;
    }
    catch (const std::exception &e)
    {
        return makeError(ErrorCode::InvalidData, "Could not load scene '" + pathStr + "': " + e.what(),
                         "SceneSerializer");
    }

    auto node = deserializeNode(j);
    auto composite = std::dynamic_pointer_cast<CompositeNode>(node);
    if (!composite)
    {
        // Wrap a non-composite root in a CompositeNode
        composite = CompositeNode::create();
        if (node)
            composite->add(node);
    }
    return composite;
}

bool SceneSerializer::save(CompositeNodePtr root, std::string_view path)
{
    return saveFile(std::move(root), path).has_value();
}

CompositeNodePtr SceneSerializer::load(std::string_view path)
{
    return loadFile(path).value_or(nullptr);
}
