#pragma once

// Turning VTK cells into triangles and lines (shared by the legacy and XML readers).

#include <ivfvis/readers.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <vector>

namespace ivfvis::detail {

/** VTK cell type numbers used here */
enum VtkCellType : int {
    VtkVertex = 1,
    VtkPolyVertex = 2,
    VtkLine = 3,
    VtkPolyLine = 4,
    VtkTriangle = 5,
    VtkTriangleStrip = 6,
    VtkPolygon = 7,
    VtkPixel = 8,
    VtkQuad = 9,
    VtkTetra = 10,
    VtkVoxel = 11,
    VtkHexahedron = 12,
    VtkWedge = 13,
    VtkPyramid = 14
};

/** Collects triangles and lines from cells; faces of volume cells shared by two cells are inner and dropped */
class CellCollector {
public:
    explicit CellCollector(VtkData &data) : m_data(data)
    {
    }

    void add(int type, std::span<const std::uint32_t> ids)
    {
        const std::size_t n = ids.size();
        switch (type)
        {
        case VtkLine:
        case VtkPolyLine:
            for (std::size_t i = 0; i + 1 < n; ++i)
                m_data.lines.emplace_back(ids[i], ids[i + 1]);
            break;
        case VtkTriangle:
        case VtkPolygon:
        case VtkQuad:
            fan(ids);
            break;
        case VtkTriangleStrip:
            for (std::size_t i = 0; i + 2 < n; ++i)
                if (i % 2 == 0)
                    triangle(ids[i], ids[i + 1], ids[i + 2]);
                else
                    triangle(ids[i + 1], ids[i], ids[i + 2]);
            break;
        case VtkPixel:
            if (n == 4)
            {
                const std::uint32_t q[4] = {ids[0], ids[1], ids[3], ids[2]};
                fan(q);
            }
            break;
        case VtkTetra:
            if (n == 4)
                faces(ids, {{{0, 1, 3, -1}, {1, 2, 3, -1}, {2, 0, 3, -1}, {0, 2, 1, -1}}}, 4);
            break;
        case VtkVoxel:
            if (n == 8)
            {
                const std::uint32_t h[8] = {ids[0], ids[1], ids[3], ids[2], ids[4], ids[5], ids[7], ids[6]};
                hexahedron(h);
            }
            break;
        case VtkHexahedron:
            if (n == 8)
                hexahedron(ids);
            break;
        case VtkWedge:
            if (n == 6)
                faces(ids, {{{0, 1, 2, -1}, {3, 5, 4, -1}, {0, 3, 4, 1}, {1, 4, 5, 2}, {2, 5, 3, 0}, {}}}, 5);
            break;
        case VtkPyramid:
            if (n == 5)
                faces(ids, {{{0, 3, 2, 1}, {0, 1, 4, -1}, {1, 2, 4, -1}, {2, 3, 4, -1}, {3, 0, 4, -1}, {}}}, 5);
            break;
        case VtkVertex:
        case VtkPolyVertex:
            break;
        default:
            ++m_skipped;
            break;
        }
    }

    /** Add the outer faces of the volume cells; returns the number of cells of unknown types */
    int finish()
    {
        for (const auto &[key, face] : m_faces)
            if (face.count == 1)
                fan(std::span<const std::uint32_t>(face.ids.data(), face.size));
        m_faces.clear();
        return m_skipped;
    }

private:
    struct Face {
        std::array<std::uint32_t, 4> ids{};
        std::size_t size{0};
        int count{0};
    };

    void triangle(std::uint32_t a, std::uint32_t b, std::uint32_t c)
    {
        if (a != b && b != c && a != c)
            m_data.triangles.emplace_back(a, b, c);
    }

    void fan(std::span<const std::uint32_t> ids)
    {
        for (std::size_t i = 1; i + 1 < ids.size(); ++i)
            triangle(ids[0], ids[i], ids[i + 1]);
    }

    using FaceList = std::array<std::array<int, 4>, 6>;

    void faces(std::span<const std::uint32_t> ids, const FaceList &list, std::size_t count)
    {
        for (std::size_t f = 0; f < count; ++f)
        {
            Face face;
            for (int k : list[f])
                if (k >= 0)
                    face.ids[face.size++] = ids[static_cast<std::size_t>(k)];
            std::array<std::uint32_t, 4> key{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
            std::copy(face.ids.begin(), face.ids.begin() + static_cast<std::ptrdiff_t>(face.size), key.begin());
            std::sort(key.begin(), key.end());
            auto [it, inserted] = m_faces.try_emplace(key, face);
            ++it->second.count;
        }
    }

    void hexahedron(std::span<const std::uint32_t> ids)
    {
        faces(ids, FaceList{{{0, 4, 7, 3}, {1, 2, 6, 5}, {0, 1, 5, 4}, {3, 7, 6, 2},
                                                      {0, 3, 2, 1}, {4, 5, 6, 7}}},
              6);
    }

    VtkData &m_data;
    std::map<std::array<std::uint32_t, 4>, Face> m_faces;
    int m_skipped{0};
};

/** Add an array of @p components values per point to @p set (1: scalars, 3: vectors, else one scalar array per component) */
inline void addPointArray(DataSet &set, const std::string &name, const std::vector<double> &values, int components)
{
    const std::size_t n = set.pointCount();
    if (components <= 0 || values.size() < n * static_cast<std::size_t>(components))
        return;
    if (components == 3)
    {
        auto v = set.addVectors(name);
        for (std::size_t i = 0; i < n; ++i)
            v[i] = glm::vec3(static_cast<float>(values[3 * i]), static_cast<float>(values[3 * i + 1]),
                             static_cast<float>(values[3 * i + 2]));
        return;
    }
    for (int c = 0; c < components; ++c)
    {
        auto s = set.addScalars(components == 1 ? name : name + "_" + std::to_string(c));
        for (std::size_t i = 0; i < n; ++i)
            s[i] = static_cast<float>(values[i * static_cast<std::size_t>(components) + static_cast<std::size_t>(c)]);
    }
}

} // namespace ivfvis::detail
