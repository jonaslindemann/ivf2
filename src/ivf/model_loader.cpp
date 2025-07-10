#include <ivf/model_loader.h>
#include <ivf/composite_node.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>

using namespace ivf;

void ModelLoader::processNode(const aiScene *scene, aiNode *node, std::shared_ptr<CompositeNode> compNode,
                              aiMatrix4x4 parentTransform)
{
    // Combine with parent transformation
    aiMatrix4x4 nodeTransform = parentTransform * node->mTransformation;

    // Extract transformation components
    aiVector3D translation, scaling;
    aiQuaternion rotation;
    nodeTransform.Decompose(scaling, rotation, translation);

    std::cout << "Node: " << node->mName.C_Str() << " - Translation: (" << translation.x << ", " << translation.y
              << ", " << translation.z << ")"
              << " - Scale: (" << scaling.x << ", " << scaling.y << ", " << scaling.z << ")" << std::endl;

    // Process all meshes in current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

        auto meshNode = MeshNode::create();
        processAiMesh(scene, mesh, meshNode);

        // Apply transformations to the mesh node
        // Convert aiQuaternion to axis-angle if your framework uses that
        float angle = 2.0f * acos(rotation.w);
        glm::vec3 axis;
        float s = sqrt(1.0f - rotation.w * rotation.w);
        if (s < 0.001f)
        {
            // If s is close to zero, direction of axis doesn't matter
            axis = glm::vec3(1.0f, 0.0f, 0.0f);
            angle = 0.0f;
        }
        else
        {
            axis = glm::vec3(rotation.x / s, rotation.y / s, rotation.z / s);
        }

        // Set transformations (assuming these methods exist on TransformNode/MeshNode)
        meshNode->setPos(glm::vec3(translation.x, translation.y, translation.z));
        meshNode->setScale(glm::vec3(scaling.x, scaling.y, scaling.z));

        // If your framework supports quaternions directly:
        // meshNode->setRotation(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));

        // Or if it uses axis-angle:
        if (angle > 0.001f)
        {
            meshNode->setRotAxis(axis);
            meshNode->setRotAngle(glm::degrees(angle));
        }

        compNode->add(meshNode);
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(scene, node->mChildren[i], compNode, nodeTransform);
    }
}

void ModelLoader::processAiMesh(const aiScene *scene, aiMesh *aiMesh, std::shared_ptr<MeshNode> meshNode)
{
    std::cout << "Processing mesh:" << std::endl;
    std::cout << "  Vertices: " << aiMesh->mNumVertices << std::endl;
    std::cout << "  Faces: " << aiMesh->mNumFaces << std::endl;

    // Count triangles
    unsigned int triangleCount = 0;
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
    {
        aiFace face = aiMesh->mFaces[i];
        if (face.mNumIndices == 3)
        {
            triangleCount++;
        }
        else if (face.mNumIndices == 4)
        {
            triangleCount += 2;
        }
        else if (face.mNumIndices > 4)
        {
            triangleCount += (face.mNumIndices - 2);
        }
    }

    std::cout << "  Triangle count after conversion: " << triangleCount << std::endl;

    // Create mesh
    meshNode->newMesh(aiMesh->mNumVertices, triangleCount);
    auto mesh = meshNode->currentMesh();

    if (!mesh)
    {
        std::cout << "Error: Failed to create mesh!" << std::endl;
        return;
    }

    // Handle normal generation based on file format and existing normals
    bool useAssimpNormals = aiMesh->HasNormals(); // Use original normals except for 3DS
    bool regenerateNormals =
        !useAssimpNormals || (std::string(scene->mRootNode->mName.C_Str()).find("bridge") != std::string::npos);

    mesh->setGenerateNormals(regenerateNormals);

    if (useAssimpNormals && !regenerateNormals)
    {
        std::cout << "  Using original normals from file" << std::endl;
    }
    else
    {
        std::cout << "  Regenerating normals with your mesh system" << std::endl;
    }

    mesh->begin(GL_TRIANGLES);

    // Process vertices
    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
    {
        mesh->vertex3f(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);

        if (aiMesh->HasNormals())
        {
            mesh->normal3f(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
        }

        if (aiMesh->HasTextureCoords(0))
        {
            mesh->tex2f(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);
        }

        if (aiMesh->HasVertexColors(0))
        {
            mesh->color4f(aiMesh->mColors[0][i].r, aiMesh->mColors[0][i].g, aiMesh->mColors[0][i].b,
                          aiMesh->mColors[0][i].a);
        }
        else
        {
            mesh->color4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    // Process faces with better winding order handling
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
    {
        aiFace face = aiMesh->mFaces[i];

        if (face.mNumIndices < 3)
        {
            continue;
        }
        else if (face.mNumIndices == 3)
        {
            // Try reversing winding order if normals appear incorrect
            mesh->index3i(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
            // mesh->index3i(face.mIndices[2], face.mIndices[1], face.mIndices[0]);
        }
        else if (face.mNumIndices == 4)
        {
            // Quad triangulation - ensure consistent winding
            mesh->index3i(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
            mesh->index3i(face.mIndices[0], face.mIndices[2], face.mIndices[3]);
        }
        else
        {
            // Polygon fan triangulation
            for (unsigned int j = 1; j < face.mNumIndices - 1; j++)
            {
                mesh->index3i(face.mIndices[0], face.mIndices[j], face.mIndices[j + 1]);
            }
        }
    }

    mesh->end();

    // Process materials (simplified to avoid potential issues)
    if (aiMesh->mMaterialIndex >= 0)
    {
        try
        {
            aiMaterial *aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
            processMaterial(aiMat, meshNode);
            meshNode->setShowNormals(true, 1.0f); // Enable normal visualization with a length of 0.1
        } catch (const std::exception &e)
        {
            std::cout << "Warning: Failed to process material: " << e.what() << std::endl;
        }
    }

    std::cout << "Mesh processed successfully with " << mesh->vertPos() << " vertices and " << mesh->indexPos()
              << " indices." << std::endl;
}

void ModelLoader::processMaterial(aiMaterial *aiMat, std::shared_ptr<MeshNode> node)
{
    aiColor3D diffuse(1.0f, 1.0f, 1.0f);
    aiColor3D ambient(0.2f, 0.2f, 0.2f);
    aiColor3D specular(1.0f, 1.0f, 1.0f);
    float shininess = 32.0f;
    float opacity = 1.0f;

    aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    aiMat->Get(AI_MATKEY_SHININESS, shininess);
    aiMat->Get(AI_MATKEY_OPACITY, opacity);

    // Debug output with higher precision
    std::cout << "Material - Diffuse: (" << std::fixed << std::setprecision(6) << diffuse.r << ", " << diffuse.g << ", "
              << diffuse.b << "), Opacity: " << opacity << std::endl;

    try
    {
        auto material = node->material();
        if (!material)
        {
            material = Material::create();
            node->setMaterial(material);
        }

        if (material)
        {
            // Ensure full precision colors
            glm::vec4 diffuseColor(diffuse.r, diffuse.g, diffuse.b, opacity);
            glm::vec4 ambientColor(ambient.r, ambient.g, ambient.b, opacity);
            glm::vec4 specularColor(specular.r, specular.g, specular.b, opacity);

            material->setDiffuseColor(diffuseColor);
            material->setAmbientColor(ambientColor);
            material->setSpecularColor(specularColor);
            material->setShininess(shininess);

            // Ensure material is properly enabled
            // Debug: Check if colors are being clamped or quantized
            std::cout << "  Set diffuse: (" << diffuseColor.r << ", " << diffuseColor.g << ", " << diffuseColor.b
                      << ", " << diffuseColor.a << ")" << std::endl;
        }
    } catch (const std::exception &e)
    {
        std::cout << "Warning: Material creation failed: " << e.what() << std::endl;
    }
}

void ModelLoader::processMaterial(aiMaterial *aiMat, std::shared_ptr<Mesh> mesh)
{
    aiColor3D diffuse(1.0f, 1.0f, 1.0f);
    aiColor3D ambient(0.2f, 0.2f, 0.2f);
    aiColor3D specular(1.0f, 1.0f, 1.0f);
    float shininess = 32.0f;
    float opacity = 1.0f;

    aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    aiMat->Get(AI_MATKEY_SHININESS, shininess);
    aiMat->Get(AI_MATKEY_OPACITY, opacity);

    // Debug output with higher precision
    std::cout << "Material - Diffuse: (" << std::fixed << std::setprecision(6) << diffuse.r << ", " << diffuse.g << ", "
              << diffuse.b << "), Opacity: " << opacity << std::endl;

    try
    {
        auto material = mesh->material();
        if (!material)
        {
            material = Material::create();
            mesh->setMaterial(material);
        }

        if (material)
        {
            // Ensure full precision colors
            glm::vec4 diffuseColor(diffuse.r, diffuse.g, diffuse.b, opacity);
            glm::vec4 ambientColor(ambient.r, ambient.g, ambient.b, opacity);
            glm::vec4 specularColor(specular.r, specular.g, specular.b, opacity);

            material->setDiffuseColor(diffuseColor);
            material->setAmbientColor(ambientColor);
            material->setSpecularColor(specularColor);
            material->setShininess(shininess);

            // Ensure material is properly enabled
            // Debug: Check if colors are being clamped or quantized
            std::cout << "  Set diffuse: (" << diffuseColor.r << ", " << diffuseColor.g << ", " << diffuseColor.b
                      << ", " << diffuseColor.a << ")" << std::endl;
        }
    } catch (const std::exception &e)
    {
        std::cout << "Warning: Material creation failed: " << e.what() << std::endl;
    }
}

std::shared_ptr<CompositeNode> ModelLoader::loadModel(const std::string &path)
{
    Assimp::Importer importer;

    unsigned int postProcessFlags =
        aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure | aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials | aiProcess_FixInfacingNormals | // This should fix inverted normals
        aiProcess_FindDegenerates | aiProcess_FindInvalidData | aiProcess_SortByPType;

    const aiScene *scene = importer.ReadFile(path, postProcessFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error("Failed to load model '" + path + "': " + std::string(importer.GetErrorString()));
    }

    std::cout << "Loaded model: " << path << std::endl;
    std::cout << "  Meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "  Materials: " << scene->mNumMaterials << std::endl;

    auto compositeNode = CompositeNode::create();
    processNode(scene, scene->mRootNode, compositeNode, aiMatrix4x4());

    std::cout << "Model loading complete. CompositeNode has " << compositeNode->count() << " children." << std::endl;

    return compositeNode;
}
