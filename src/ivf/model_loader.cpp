#include <ivf/model_loader.h>
#include <ivf/composite_node.h>
#include <ivf/logger.h>

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

    logInfofc("ModelLoader",
               "Processing node: %s | Translation: (%.2f, %.2f, %.2f) | Scale: (%.2f, %.2f, %.2f)",
               node->mName.C_Str(), translation.x, translation.y, translation.z, scaling.x, scaling.y, scaling.z);

    // Process meshes in current node
    if (node->mNumMeshes > 0)
    {
        auto meshNode = MeshNode::create();
        meshNode->setName(node->mName.C_Str());
        meshNode->setPos(glm::vec3(translation.x, translation.y, translation.z));
        meshNode->setScale(glm::vec3(scaling.x, scaling.y, scaling.z));

        float angle = 2.0f * acos(rotation.w);
        glm::vec3 axis;
        float s = sqrt(1.0f - rotation.w * rotation.w);
        if (s < 0.001f)
        {
            axis = glm::vec3(1.0f, 0.0f, 0.0f);
            angle = 0.0f;
        }
        else
        {
            axis = glm::vec3(rotation.x / s, rotation.y / s, rotation.z / s);
        }

        if (angle > 0.001f)
        {
            meshNode->setRotAxis(axis);
            meshNode->setRotAngle(glm::degrees(angle));
        }

        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            processAiMesh(scene, mesh, meshNode);
        }
        compNode->add(meshNode);
    }

    // Process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        aiNode *childNode = node->mChildren[i];

        // Count total meshes in this child and all its descendants
        std::function<int(aiNode *)> countMeshes = [&](aiNode *n) -> int {
            int count = n->mNumMeshes;
            for (unsigned int j = 0; j < n->mNumChildren; j++)
            {
                count += countMeshes(n->mChildren[j]);
            }
            return count;
        };

        int totalMeshesInChild = countMeshes(childNode);

        if (totalMeshesInChild <= 1)
        {
            // Child has 0 or 1 mesh - process directly into current composite (no intermediate composite)
            processNode(scene, childNode, compNode, nodeTransform);
        }
        else
        {
            // Child has multiple meshes - create a composite for it
            std::shared_ptr<CompositeNode> childCompNode = CompositeNode::create();
            childCompNode->setName(std::string(childNode->mName.C_Str()));
            compNode->add(childCompNode);
            processNode(scene, childNode, childCompNode, nodeTransform);
        }
    }
}
void ModelLoader::processAiMesh(const aiScene *scene, aiMesh *aiMesh, std::shared_ptr<MeshNode> meshNode)
{
    logInfo("Processing aiMesh...", "ModelLoader");
    logInfofc("ModelLoader", "  Mesh name: {}", aiMesh->mName.C_Str());
    logInfofc("ModelLoader", "  Material index: {}", aiMesh->mMaterialIndex);
    logInfofc("ModelLoader", "  Vertices: {}", aiMesh->mNumVertices);
    logInfofc("ModelLoader", "  Faces: {}", aiMesh->mNumFaces);

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

    logInfofc("ModelLoader", "  Original face count: {}", aiMesh->mNumFaces);
    logInfofc("ModelLoader", "  Triangle count after conversion: {}", triangleCount);

    // Create mesh
    meshNode->newMesh(aiMesh->mNumVertices, triangleCount);
    auto mesh = meshNode->currentMesh();

    if (!mesh)
    {
        logError("Failed to create mesh!", "ModelLoader");
        return;
    }

    // Handle normal generation based on file format and existing normals
    bool useAssimpNormals = aiMesh->HasNormals(); // Use original normals except for 3DS

    mesh->setGenerateNormals(!useAssimpNormals);

    if (useAssimpNormals)
    {
        logInfo("  Using original normals from file", "ModelLoader");
    }
    else
    {
        logInfo("  No normals found or regenerating normals", "ModelLoader");
    }

    mesh->begin(GL_TRIANGLES);

    // Process vertices
    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
    {
        mesh->vertex3f(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);

        if (aiMesh->HasNormals())
        {
            glm::vec3 normal(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);

            // Test both directions:
            mesh->normal3f(normal.x, normal.y, normal.z); // Original
            // mesh->normal3f(-normal.x, -normal.y, -normal.z); // Flipped        }
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

    // Process faces with adaptive winding order
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
    {
        aiFace face = aiMesh->mFaces[i];

        if (face.mNumIndices == 3)
        {
            // Get vertices and calculate face normal
            aiVector3D v0 = aiMesh->mVertices[face.mIndices[0]];
            aiVector3D v1 = aiMesh->mVertices[face.mIndices[1]];
            aiVector3D v2 = aiMesh->mVertices[face.mIndices[2]];

            // Calculate face normal using cross product
            aiVector3D edge1 = v1 - v0;
            aiVector3D edge2 = v2 - v0;
            aiVector3D faceNormal = edge1 ^ edge2; // Cross product
            faceNormal.Normalize();

            // Get vertex normal (if available)
            if (aiMesh->HasNormals())
            {
                aiVector3D vertexNormal = aiMesh->mNormals[face.mIndices[0]];

                // Check if face normal and vertex normal agree
                float dot = faceNormal * vertexNormal; // Dot product

                if (dot > 0)
                {
                    // Normals agree - use original winding
                    mesh->index3i(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
                }
                else
                {
                    // Normals disagree - flip winding
                    mesh->index3i(face.mIndices[2], face.mIndices[1], face.mIndices[0]);
                }
            }
            else
            {
                // No vertex normals - use your current global flip
                mesh->index3i(face.mIndices[2], face.mIndices[1], face.mIndices[0]);
            }
        }
    }

    mesh->end();

    // Update bounding box after mesh creation
    meshNode->updateBoundingBox();
    
    // Debug: Print local bounding box information
    auto localBbox = meshNode->localBoundingBox();
    if (localBbox.isValid())
    {
        logInfo("  Local bounding box:", "ModelLoader");
        logInfofc("ModelLoader", "    Min: ({}, {}, {})", localBbox.min().x, localBbox.min().y, localBbox.min().z);
        logInfofc("ModelLoader", "    Max: ({}, {}, {})", localBbox.max().x, localBbox.max().y, localBbox.max().z);
        logInfofc("ModelLoader", "    Size: ({}, {}, {})", localBbox.size().x, localBbox.size().y, localBbox.size().z);
    }
    else
    {
        logWarning("  Local bounding box is invalid!", "ModelLoader");
    }

    // Process materials (simplified to avoid potential issues)
    if (aiMesh->mMaterialIndex >= 0)
    {
        try
        {
            aiMaterial *aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
            processMaterial(aiMat, meshNode);
            // meshNode->setShowNormals(true, 0.1f); // Enable normal visualization with a length of 0.1
        } catch (const std::exception &e)
        {
            logWarningfc("Failed to process material: {}", e.what(), "ModelLoader");
        }
    }

    logInfofc("ModelLoader", "  Mesh processed successfully with {} vertices and {} indices.", mesh->vertPos(),
              mesh->indexPos());
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

    logInfofc("ModelLoader",
               "Material - Diffuse: ({:.6f}, {:.6f}, {:.6f}), Opacity: {:.6f}", diffuse.r, diffuse.g,
              diffuse.b, opacity);

    logInfofc("ModelLoader", "  Ambient: ({:.6f}, {:.6f}, {:.6f})", ambient.r, ambient.g, ambient.b);
    logInfofc("ModelLoader", "  Specular: ({:.6f}, {:.6f}, {:.6f})", specular.r, specular.g, specular.b);
    logInfofc("ModelLoader", "  Shininess: {:.6f}", shininess);
    logInfofc("ModelLoader", "  Opacity: {:.6f}", opacity);

    try
    {
        auto material = node->material();
        if (!material)
        {
            material = Material::create();
            /*
            material->setDiffuseColor(glm::vec4(0.6, 0.6, 0.6, 1.0));
            material->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
            material->setAmbientColor(glm::vec4(0.2, 0.2, 0.2, 1.0));
            */
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

            logInfofc("ModelLoader", "  Set diffuse: ({:.6f}, {:.6f}, {:.6f}, {:.6f})", diffuseColor.r, diffuseColor.g,
                      diffuseColor.b, diffuseColor.a);
        }
    } catch (const std::exception &e)
    {
        logWarningfc("Material creation failed: {}", e.what(), "ModelLoader");  
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

    logInfofc("ModelLoader",
               "Material - Diffuse: ({:.6f}, {:.6f}, {:.6f}), Opacity: {:.6f}", diffuse.r, diffuse.g,
              diffuse.b, opacity);

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

            logInfofc("ModelLoader", "  Set ambient: ({:.6f}, {:.6f}, {:.6f}, {:.6f})", ambientColor.r, ambientColor.g,
                      ambientColor.b, ambientColor.a);
        }
    } catch (const std::exception &e)
    {
        logWarningfc("Material creation failed: {}", e.what(), "ModelLoader");
    }
}

std::shared_ptr<CompositeNode> ModelLoader::loadModel(const std::string &path)
{
    Assimp::Importer importer;

    importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);

    unsigned int postProcessFlags =
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals;

    const aiScene *scene = importer.ReadFile(path, postProcessFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error("Failed to load model '" + path + "': " + std::string(importer.GetErrorString()));
    }

    logInfofc("ModelLoader", "Successfully loaded model: {}", path);
    logInfofc("ModelLoader", "  Meshes: {}", scene->mNumMeshes);
    logInfofc("ModelLoader", "  Materials: {}", scene->mNumMaterials);

    auto compositeNode = CompositeNode::create();
    processNode(scene, scene->mRootNode, compositeNode, aiMatrix4x4());

    logInfofc("ModelLoader", "Model loading complete. CompositeNode has {} children.", compositeNode->count());

    // If the top-level composite only has one child and that child is also a composite,
    // return the child directly to eliminate unnecessary nesting
    if (compositeNode->count() == 1)
    {
        auto firstChild = std::dynamic_pointer_cast<CompositeNode>(compositeNode->at(0));
        if (firstChild)
        {
            logInfo("Eliminating top-level composite wrapper", "ModelLoader");
            return firstChild;
        }
    }

    return compositeNode;
}
