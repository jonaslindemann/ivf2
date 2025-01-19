#include <ivf/model_loader.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stdexcept>

using namespace ivf;

void ModelLoader::processNode(const aiScene *scene, aiNode *node, std::shared_ptr<MeshNode> meshNode)
{
    // Process all meshes in current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processAiMesh(scene, mesh, meshNode);
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(scene, node->mChildren[i], meshNode);
    }
}

void ModelLoader::processAiMesh(const aiScene *scene, aiMesh *aiMesh, std::shared_ptr<MeshNode> meshNode)
{
    // Create new mesh with appropriate size
    meshNode->newMesh(aiMesh->mNumVertices, aiMesh->mNumFaces);
    auto mesh = meshNode->currentMesh();

    mesh->begin(GL_TRIANGLES);

    // Process vertices
    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
    {
        // Vertices
        mesh->vertex3f(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);

        // Normals (if present)
        if (aiMesh->HasNormals())
        {
            mesh->normal3f(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
        }

        // Texture coordinates (if present)
        if (aiMesh->HasTextureCoords(0))
        {
            mesh->tex2f(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);
        }

        // Colors (if present)
        if (aiMesh->HasVertexColors(0))
        {
            mesh->color4f(aiMesh->mColors[0][i].r, aiMesh->mColors[0][i].g, aiMesh->mColors[0][i].b,
                          aiMesh->mColors[0][i].a);
        }
    }

    // Process indices
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
    {
        aiFace face = aiMesh->mFaces[i];
        // Assimp triangulates meshes by default with aiProcess_Triangulate
        mesh->index3i(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
    }

    mesh->end();

    // Process materials if present
    if (aiMesh->mMaterialIndex >= 0)
    {
        // Here you would process materials based on your Material class
        // aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
        // ... process material properties ...
    }
}

std::shared_ptr<MeshNode> ModelLoader::loadModel(const std::string &path)
{
    Assimp::Importer importer;

    // Common post-processing steps
    unsigned int postProcessFlags = aiProcess_Triangulate |     // Convert all primitive shapes to triangles
                                    aiProcess_GenNormals |      // Generate normals if not present
                                    aiProcess_GenUVCoords |     // Generate texture coordinates if not present
                                    aiProcess_FlipUVs |         // Flip texture coordinates on y-axis
                                    aiProcess_CalcTangentSpace; // Calculate tangent space for normal mapping

    const aiScene *scene = importer.ReadFile(path, postProcessFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
    }

    auto meshNode = MeshNode::create();
    processNode(scene, scene->mRootNode, meshNode);

    return meshNode;
}
