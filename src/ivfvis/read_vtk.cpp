// Legacy VTK reader (.vtk), the readVtk() dispatcher and createMeshNode()

#include <ivfvis/readers.h>

#include "vtk_cells.h"

#include <ivf/extrusion_builder.h>
#include <ivf/logger.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

using namespace ivfvis;

namespace ivfvis::detail {
// read_vtk_xml.cpp
ivf::Result<VtkData> readVtkXml(const std::filesystem::path &path, const std::string &bytes);
} // namespace ivfvis::detail

namespace {

constexpr const char *Context = "Readers";

std::string lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

/** Size in bytes of a legacy VTK data type (0 if unknown) */
std::size_t typeSize(const std::string &type)
{
    const std::string t = lower(type);
    if (t == "unsigned_char" || t == "char" || t == "vtktypeint8" || t == "vtktypeuint8" || t == "bit")
        return 1;
    if (t == "unsigned_short" || t == "short" || t == "vtktypeint16" || t == "vtktypeuint16")
        return 2;
    if (t == "unsigned_int" || t == "int" || t == "float" || t == "vtktypeint32" || t == "vtktypeuint32" ||
        t == "vtkidtype")
        return 4;
    if (t == "unsigned_long" || t == "long" || t == "double" || t == "vtktypeint64" || t == "vtktypeuint64" ||
        t == "unsigned_long_long" || t == "long_long")
        return 8;
    return 0;
}

/** A cursor over a legacy VTK file */
class Parser {
public:
    Parser(std::string text, std::filesystem::path path) : m_s(std::move(text)), m_path(std::move(path))
    {
    }

    bool binary{false};
    int version{2};

    [[nodiscard]] bool atEnd()
    {
        skipSpace();
        return m_pos >= m_s.size();
    }

    std::string word()
    {
        skipSpace();
        const std::size_t start = m_pos;
        while (m_pos < m_s.size() && !std::isspace(static_cast<unsigned char>(m_s[m_pos])))
            ++m_pos;
        return m_s.substr(start, m_pos - start);
    }

    std::string peekWord()
    {
        const std::size_t saved = m_pos;
        std::string w = word();
        m_pos = saved;
        return w;
    }

    /** The rest of the current line (without the line break) */
    std::string restOfLine()
    {
        const std::size_t start = m_pos;
        while (m_pos < m_s.size() && m_s[m_pos] != '\n')
            ++m_pos;
        std::string line = m_s.substr(start, m_pos - start);
        if (m_pos < m_s.size())
            ++m_pos;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        return line;
    }

    long long integer()
    {
        const std::string w = word();
        return std::atoll(w.c_str());
    }

    /** @p count values of @p type: text in ASCII files, big-endian binary after a line break otherwise */
    bool values(std::size_t count, const std::string &type, std::vector<double> &out)
    {
        out.resize(count);
        if (!binary)
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                const std::string w = word();
                if (w.empty())
                    return false;
                out[i] = std::strtod(w.c_str(), nullptr);
            }
            return true;
        }
        const std::size_t size = typeSize(type);
        if (size == 0)
            return false;
        // Binary data starts after the line break that ends the header line
        restOfLine();
        if (m_pos + count * size > m_s.size())
            return false;
        const std::string t = lower(type);
        const bool isFloat = t == "float" || t == "double";
        const bool isSigned = t.find("unsigned") == std::string::npos && t.find("uint") == std::string::npos;
        unsigned char raw[8];
        for (std::size_t i = 0; i < count; ++i)
        {
            std::memcpy(raw, m_s.data() + m_pos + i * size, size);
            std::reverse(raw, raw + size); // big endian
            if (isFloat && size == 4)
            {
                float f;
                std::memcpy(&f, raw, 4);
                out[i] = f;
            }
            else if (isFloat)
                std::memcpy(&out[i], raw, 8);
            else
            {
                std::uint64_t u = 0;
                for (int k = static_cast<int>(size) - 1; k >= 0; --k)
                    u = (u << 8) | raw[k];
                const int bits = static_cast<int>(8 * size);
                if (isSigned && bits < 64 && ((u >> (bits - 1)) & 1u))
                    u |= ~std::uint64_t(0) << bits;
                out[i] = isSigned ? static_cast<double>(static_cast<std::int64_t>(u)) : static_cast<double>(u);
            }
        }
        m_pos += count * size;
        return true;
    }

    /** Skip a METADATA block (up to the next empty line) */
    void skipMetadata()
    {
        restOfLine();
        while (m_pos < m_s.size())
        {
            const std::string line = restOfLine();
            if (line.find_first_not_of(" \t") == std::string::npos)
                break;
        }
    }

    std::unexpected<ivf::Error> fail(const std::string &why) const
    {
        return ivf::makeError(ivf::ErrorCode::InvalidData,
                              "Could not read VTK file '" + m_path.string() + "': " + why, Context);
    }

private:
    void skipSpace()
    {
        while (m_pos < m_s.size() && std::isspace(static_cast<unsigned char>(m_s[m_pos])))
            ++m_pos;
    }

    std::string m_s;
    std::size_t m_pos{0};
    std::filesystem::path m_path;
};

/** Cells as offsets and connectivity */
struct Cells {
    std::vector<std::uint32_t> offsets{0};
    std::vector<std::uint32_t> connectivity;

    [[nodiscard]] std::size_t count() const
    {
        return offsets.size() - 1;
    }
    [[nodiscard]] std::span<const std::uint32_t> cell(std::size_t i) const
    {
        return std::span<const std::uint32_t>(connectivity).subspan(offsets[i], offsets[i + 1] - offsets[i]);
    }
};

/** Cells after a POLYGONS/CELLS/... keyword: "n size" + records (< 5.0) or OFFSETS + CONNECTIVITY (5.x) */
bool readCells(Parser &p, Cells &cells)
{
    const long long a = p.integer(), b = p.integer();
    std::vector<double> values;
    if (p.version >= 5)
    {
        if (p.word() != "OFFSETS")
            return false;
        std::string type = p.word();
        if (!p.values(static_cast<std::size_t>(a), type, values))
            return false;
        std::vector<std::uint32_t> offsets;
        for (double v : values)
            offsets.push_back(static_cast<std::uint32_t>(v));
        if (p.word() != "CONNECTIVITY")
            return false;
        type = p.word();
        if (!p.values(static_cast<std::size_t>(b), type, values))
            return false;
        const auto base = static_cast<std::uint32_t>(cells.connectivity.size());
        for (std::size_t i = 1; i < offsets.size(); ++i)
            cells.offsets.push_back(base + offsets[i]);
        for (double v : values)
            cells.connectivity.push_back(static_cast<std::uint32_t>(v));
        return true;
    }
    if (!p.values(static_cast<std::size_t>(b), "int", values))
        return false;
    std::size_t k = 0;
    for (long long c = 0; c < a && k < values.size(); ++c)
    {
        const auto n = static_cast<std::size_t>(values[k++]);
        for (std::size_t j = 0; j < n && k < values.size(); ++j)
            cells.connectivity.push_back(static_cast<std::uint32_t>(values[k++]));
        cells.offsets.push_back(static_cast<std::uint32_t>(cells.connectivity.size()));
    }
    return true;
}

std::string decodeName(std::string name)
{
    // Spaces are written as %20
    std::string out;
    for (std::size_t i = 0; i < name.size(); ++i)
        if (name[i] == '%' && i + 2 < name.size())
        {
            out += static_cast<char>(std::strtol(name.substr(i + 1, 2).c_str(), nullptr, 16));
            i += 2;
        }
        else
            out += name[i];
    return out;
}

/** Attributes after POINT_DATA/CELL_DATA, stored into @p set (nullptr: read and drop) */
bool readAttributes(Parser &p, std::size_t n, DataSet *set)
{
    std::vector<double> values;
    while (!p.atEnd())
    {
        const std::string key = p.peekWord();
        const std::string k = lower(key);
        if (k == "point_data" || k == "cell_data")
            return true;
        p.word();
        if (k == "scalars")
        {
            const std::string name = decodeName(p.word());
            const std::string type = p.word();
            std::string rest = p.restOfLine();
            const int components = std::max(1, std::atoi(rest.c_str()));
            if (lower(p.peekWord()) == "lookup_table")
            {
                p.word();
                p.word();
            }
            if (!p.values(n * static_cast<std::size_t>(components), type, values))
                return false;
            if (set)
                detail::addPointArray(*set, name, values, components);
        }
        else if (k == "vectors" || k == "normals")
        {
            const std::string name = decodeName(p.word());
            const std::string type = p.word();
            if (!p.values(n * 3, type, values))
                return false;
            if (set)
                detail::addPointArray(*set, name, values, 3);
        }
        else if (k == "texture_coordinates")
        {
            p.word();
            const auto dim = static_cast<std::size_t>(p.integer());
            const std::string type = p.word();
            if (!p.values(n * dim, type, values))
                return false;
        }
        else if (k == "tensors" || k == "tensors6")
        {
            p.word();
            const std::string type = p.word();
            if (!p.values(n * (k == "tensors" ? 9 : 6), type, values))
                return false;
        }
        else if (k == "color_scalars")
        {
            p.word();
            const auto count = static_cast<std::size_t>(p.integer());
            if (!p.values(n * count, p.binary ? "unsigned_char" : "float", values))
                return false;
        }
        else if (k == "lookup_table")
        {
            p.word();
            const auto count = static_cast<std::size_t>(p.integer());
            if (!p.values(count * 4, p.binary ? "unsigned_char" : "float", values))
                return false;
        }
        else if (k == "field")
        {
            p.word();
            const long long arrays = p.integer();
            for (long long a = 0; a < arrays; ++a)
            {
                if (lower(p.peekWord()) == "metadata")
                {
                    p.skipMetadata();
                    --a;
                    continue;
                }
                const std::string name = decodeName(p.word());
                const auto components = static_cast<int>(p.integer());
                const auto tuples = static_cast<std::size_t>(p.integer());
                const std::string type = p.word();
                if (!p.values(tuples * static_cast<std::size_t>(components), type, values))
                    return false;
                if (set && tuples == n)
                    detail::addPointArray(*set, name, values, components);
            }
        }
        else if (k == "metadata")
            p.skipMetadata();
        else
            return false; // unknown keyword
    }
    return true;
}

ivf::Result<VtkData> readLegacy(const std::filesystem::path &path, std::string text)
{
    Parser p(std::move(text), path);
    const std::string first = p.restOfLine();
    if (first.find("vtk DataFile Version") == std::string::npos)
        return ivf::makeError(ivf::ErrorCode::UnsupportedFormat,
                              "Could not read VTK file '" + path.string() + "': not a legacy VTK file", Context);
    p.version = std::atoi(first.c_str() + first.find("Version") + 8);
    p.restOfLine(); // title
    const std::string format = lower(p.word());
    if (format != "ascii" && format != "binary")
        return p.fail("expected ASCII or BINARY");
    p.binary = format == "binary";
    p.restOfLine();

    VtkData data;
    std::string datasetType;
    glm::ivec3 dims(1);
    glm::vec3 origin(0.0f), spacing(1.0f);
    std::vector<glm::vec3> positions;
    std::vector<double> values;
    std::array<std::vector<double>, 3> coordinates;
    Cells polys, lines, strips, cells;
    std::vector<int> cellTypes;
    bool pointDataRead = false;

    auto makePoints = [&]() {
        if (!data.points)
            data.points = PointSet::create(positions);
    };

    while (!p.atEnd())
    {
        const std::string key = lower(p.word());
        if (key == "dataset")
            datasetType = lower(p.word());
        else if (key == "dimensions")
            for (int a = 0; a < 3; ++a)
                dims[a] = static_cast<int>(p.integer());
        else if (key == "spacing" || key == "aspect_ratio")
            for (int a = 0; a < 3; ++a)
                spacing[a] = static_cast<float>(std::strtod(p.word().c_str(), nullptr));
        else if (key == "origin")
            for (int a = 0; a < 3; ++a)
                origin[a] = static_cast<float>(std::strtod(p.word().c_str(), nullptr));
        else if (key == "points")
        {
            const auto n = static_cast<std::size_t>(p.integer());
            const std::string type = p.word();
            if (!p.values(n * 3, type, values))
                return p.fail("truncated POINTS");
            positions.resize(n);
            for (std::size_t i = 0; i < n; ++i)
                positions[i] = glm::vec3(static_cast<float>(values[3 * i]), static_cast<float>(values[3 * i + 1]),
                                         static_cast<float>(values[3 * i + 2]));
        }
        else if (key == "x_coordinates" || key == "y_coordinates" || key == "z_coordinates")
        {
            const auto n = static_cast<std::size_t>(p.integer());
            const std::string type = p.word();
            if (!p.values(n, type, coordinates[static_cast<std::size_t>(key[0] - 'x')]))
                return p.fail("truncated coordinates");
        }
        else if (key == "polygons" || key == "lines" || key == "triangle_strips" || key == "vertices" ||
                 key == "cells")
        {
            Cells *target = key == "polygons" ? &polys : key == "lines" ? &lines : key == "triangle_strips" ? &strips
                          : key == "cells"    ? &cells
                                              : nullptr;
            Cells ignored;
            if (!readCells(p, target ? *target : ignored))
                return p.fail("malformed " + key);
        }
        else if (key == "cell_types")
        {
            const auto n = static_cast<std::size_t>(p.integer());
            if (!p.values(n, "int", values))
                return p.fail("truncated CELL_TYPES");
            cellTypes.clear();
            for (double v : values)
                cellTypes.push_back(static_cast<int>(v));
        }
        else if (key == "point_data" || key == "cell_data")
        {
            const auto n = static_cast<std::size_t>(p.integer());
            DataSet *set = nullptr;
            if (key == "point_data" && !pointDataRead)
            {
                pointDataRead = true;
                if (datasetType == "structured_points")
                {
                    data.grid = UniformGrid::create(dims, origin, spacing);
                    set = data.grid.get();
                }
                else
                {
                    if (datasetType == "rectilinear_grid" && positions.empty())
                        for (std::size_t k = 0; k < coordinates[2].size(); ++k)
                            for (std::size_t j = 0; j < coordinates[1].size(); ++j)
                                for (std::size_t i = 0; i < coordinates[0].size(); ++i)
                                    positions.emplace_back(static_cast<float>(coordinates[0][i]), static_cast<float>(coordinates[1][j]),
                                                           static_cast<float>(coordinates[2][k]));
                    makePoints();
                    set = data.points.get();
                }
                if (set->pointCount() != n)
                    set = nullptr; // inconsistent: read but drop
            }
            if (!readAttributes(p, n, set))
                return p.fail("malformed " + key);
        }
        else if (key == "field")
        {
            // Field data of the data set (time values and the like)
            p.word();
            const long long arrays = p.integer();
            for (long long a = 0; a < arrays; ++a)
            {
                if (lower(p.peekWord()) == "metadata")
                {
                    p.skipMetadata();
                    --a;
                    continue;
                }
                p.word();
                const auto components = static_cast<std::size_t>(p.integer());
                const auto tuples = static_cast<std::size_t>(p.integer());
                const std::string type = p.word();
                if (!p.values(components * tuples, type, values))
                    return p.fail("malformed FIELD");
            }
        }
        else if (key == "metadata")
            p.skipMetadata();
        else
            return p.fail("unexpected keyword '" + key + "'");
    }

    if (datasetType == "structured_points")
    {
        if (!data.grid)
            data.grid = UniformGrid::create(dims, origin, spacing);
        return data;
    }
    if (datasetType == "rectilinear_grid" && positions.empty())
        for (std::size_t k = 0; k < coordinates[2].size(); ++k)
            for (std::size_t j = 0; j < coordinates[1].size(); ++j)
                for (std::size_t i = 0; i < coordinates[0].size(); ++i)
                    positions.emplace_back(static_cast<float>(coordinates[0][i]), static_cast<float>(coordinates[1][j]),
                                                           static_cast<float>(coordinates[2][k]));
    makePoints();

    detail::CellCollector collector(data);
    for (std::size_t i = 0; i < polys.count(); ++i)
        collector.add(detail::VtkPolygon, polys.cell(i));
    for (std::size_t i = 0; i < strips.count(); ++i)
        collector.add(detail::VtkTriangleStrip, strips.cell(i));
    for (std::size_t i = 0; i < lines.count(); ++i)
        collector.add(detail::VtkPolyLine, lines.cell(i));
    for (std::size_t i = 0; i < cells.count() && i < cellTypes.size(); ++i)
        collector.add(cellTypes[i], cells.cell(i));
    if (const int skipped = collector.finish(); skipped > 0)
        ivf::logWarning(ivf::logStr(skipped, " cells of unsupported types in '", path.string(), "' left out"), Context);
    return data;
}

} // namespace

ivf::Result<VtkData> ivfvis::readVtk(const std::filesystem::path &path)
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec))
        return ivf::makeError(ivf::ErrorCode::FileNotFound,
                              "Could not read VTK file '" + path.string() + "': file not found", Context);
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return ivf::makeError(ivf::ErrorCode::ReadFailed,
                              "Could not read VTK file '" + path.string() + "': cannot open it", Context);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    std::string bytes = buffer.str();

    if (bytes.find("<VTKFile") != std::string::npos && bytes.find("<VTKFile") < 512)
        return detail::readVtkXml(path, bytes);
    return readLegacy(path, std::move(bytes));
}

ivf::MeshNodePtr ivfvis::createMeshNode(const PointSet &points, std::span<const glm::uvec3> triangles)
{
    const std::size_t n = points.pointCount();
    ivf::MeshData data;
    data.positions.resize(n);
    data.normals.assign(n, glm::vec3(0.0f));
    for (std::size_t i = 0; i < n; ++i)
        data.positions[i] = points.position(i);
    for (const glm::uvec3 &t : triangles)
    {
        if (t.x >= n || t.y >= n || t.z >= n)
            continue;
        // Area-weighted normals
        const glm::vec3 normal = glm::cross(data.positions[t.y] - data.positions[t.x],
                                            data.positions[t.z] - data.positions[t.x]);
        data.normals[t.x] += normal;
        data.normals[t.y] += normal;
        data.normals[t.z] += normal;
        data.indices.push_back(t);
    }
    for (glm::vec3 &normal : data.normals)
        normal = glm::length(normal) > 0.0f ? glm::normalize(normal) : glm::vec3(0.0f, 1.0f, 0.0f);
    data.texCoords.assign(n, glm::vec2(0.0f));
    data.colors.assign(n, glm::vec4(1.0f));
    auto node = ivf::MeshNode::create();
    node->createFromMeshData(data);
    return node;
}
