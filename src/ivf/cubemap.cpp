#include <ivf/cubemap.h>

#include <ivf/logger.h>

#include <stb_image.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>

namespace fs = std::filesystem;
using namespace ivf;

Cubemap::Cubemap()
{
    glGenTextures(1, &m_id);
}

Cubemap::~Cubemap()
{
    if (m_id) glDeleteTextures(1, &m_id);
}

std::shared_ptr<Cubemap> Cubemap::create()
{
    return std::make_shared<Cubemap>();
}

bool Cubemap::loadFace(GLenum target, std::string_view path)
{
    int w, h, ch;
    unsigned char* data = stbi_load(std::string(path).c_str(), &w, &h, &ch, 0);
    if (!data) {
        logErrorfc("Cubemap", "Cannot load face: {}", path);
        return false;
    }

    // OpenGL's cubemap face coordinates are left-handed relative to the
    // right-handed camera/world convention used by the scene. Mirror each
    // source face horizontally so labeled skybox images read naturally.
    for (int y = 0; y < h; ++y) {
        auto* row = data + y * w * ch;
        for (int x = 0; x < w / 2; ++x) {
            auto* left = row + x * ch;
            auto* right = row + (w - 1 - x) * ch;
            for (int c = 0; c < ch; ++c)
                std::swap(left[c], right[c]);
        }
    }

    GLenum fmt = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB : GL_RED;
    glTexImage2D(target, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return true;
}

bool Cubemap::load(std::string_view posX, std::string_view negX,
                   std::string_view posY, std::string_view negY,
                   std::string_view posZ, std::string_view negZ)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    bool ok = true;
    ok &= loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, posX);
    ok &= loadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, negX);
    ok &= loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, posY);
    ok &= loadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, negY);
    ok &= loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, posZ);
    ok &= loadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, negZ);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return ok;
}

std::string Cubemap::findFile(std::string_view dir, std::string_view stem)
{
    for (auto& ext : {".jpg", ".jpeg", ".png", ".hdr", ".tga"}) {
        auto p = fs::path(dir) / (std::string(stem) + ext);
        if (fs::exists(p)) return p.string();
    }
    return {};
}

bool Cubemap::loadFromDirectory(std::string_view dir)
{
    return load(
        findFile(dir, "right"),  findFile(dir, "left"),
        findFile(dir, "top"),    findFile(dir, "bottom"),
        findFile(dir, "back"),   findFile(dir, "front"));
}

bool Cubemap::loadGradient(glm::vec3 topColor, glm::vec3 horizonColor, glm::vec3 bottomColor, int size)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    std::vector<unsigned char> face(size * size * 3);

    // UV [0,1] -> NDC [-1,1] direction vectors per cubemap face.
    // face_dir(face, s, t) returns the unnormalized ray direction.
    auto faceDir = [](int face, float s, float t) -> glm::vec3 {
        // s, t in [-1, 1]
        switch (face) {
            case 0: return { 1,  t, -s}; // +X
            case 1: return {-1,  t,  s}; // -X
            case 2: return { s,  1, -t}; // +Y
            case 3: return { s, -1,  t}; // -Y
            case 4: return { s,  t,  1}; // +Z
            default:return {-s,  t, -1}; // -Z
        }
    };

    for (int f = 0; f < 6; ++f) {
        for (int row = 0; row < size; ++row) {
            for (int col = 0; col < size; ++col) {
                float s = (col + 0.5f) / size * 2.0f - 1.0f;
                float t = 1.0f - (row + 0.5f) / size * 2.0f; // flip V
                glm::vec3 dir = glm::normalize(faceDir(f, s, t));

                // y in [-1, 1]: map to [0, 1] for mixing
                float y = dir.y;
                glm::vec3 color;
                if (y >= 0.0f) {
                    // horizon → top
                    float blend = std::pow(y, 0.5f); // gentle curve
                    color = glm::mix(horizonColor, topColor, blend);
                } else {
                    // horizon → bottom
                    float blend = std::pow(-y, 0.5f);
                    color = glm::mix(horizonColor, bottomColor, blend);
                }

                int idx = (row * size + col) * 3;
                face[idx + 0] = static_cast<unsigned char>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
                face[idx + 1] = static_cast<unsigned char>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
                face[idx + 2] = static_cast<unsigned char>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
            }
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGB, size, size, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, face.data());
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return true;
}

void Cubemap::bind(int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void Cubemap::unbind()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
