// VTK XML reader (.vtu, .vtp, .vti): ASCII, base64 and appended data, raw or zlib-compressed

#include <ivfvis/readers.h>

#include "vtk_cells.h"

#include <ivf/logger.h>

#ifdef IVFVIS_WITH_VTK_XML
#include <miniz.h>
#include <pugixml.hpp>
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>

using namespace ivfvis;

namespace ivfvis::detail {
ivf::Result<VtkData> readVtkXml(const std::filesystem::path &path, const std::string &bytes);
}

#ifndef IVFVIS_WITH_VTK_XML

ivf::Result<VtkData> ivfvis::detail::readVtkXml(const std::filesystem::path &path, const std::string & /*bytes*/)
{
    return ivf::makeError(ivf::ErrorCode::NotSupported,
                          "Could not read VTK XML file '" + path.string() +
                              "': ivf2 was built without VTK XML support (IVF2_WITH_VTK_XML=OFF)",
                          "Readers");
}

#else

namespace {

constexpr const char *Context = "Readers";

std::string decodeBase64(std::string_view text)
{
    auto value = [](char c) -> int {
        if (c >= 'A' && c <= 'Z')
            return c - 'A';
        if (c >= 'a' && c <= 'z')
            return c - 'a' + 26;
        if (c >= '0' && c <= '9')
            return c - '0' + 52;
        if (c == '+')
            return 62;
        if (c == '/')
            return 63;
        return -1;
    };
    std::string out;
    unsigned buffer = 0;
    int bits = 0;
    for (char c : text)
    {
        if (c == '=')
        {
            // End of one encoded block (VTK concatenates blocks): drop partial bits
            buffer = 0;
            bits = 0;
            continue;
        }
        const int v = value(c);
        if (v < 0)
            continue;
        buffer = (buffer << 6) | static_cast<unsigned>(v);
        bits += 6;
        if (bits >= 8)
        {
            bits -= 8;
            out += static_cast<char>((buffer >> bits) & 0xff);
        }
    }
    return out;
}

/** Settings of the file that affect binary data */
struct Format {
    bool bigEndian{false};
    std::size_t headerSize{4};
    bool compressed{false};
};

std::uint64_t readUInt(const unsigned char *p, std::size_t size, bool bigEndian)
{
    std::uint64_t v = 0;
    for (std::size_t i = 0; i < size; ++i)
    {
        const std::size_t k = bigEndian ? i : size - 1 - i;
        v = (v << 8) | p[k];
    }
    return v;
}

/** Size of the binary header at @p p: one size, or (blocks, block size, last size, compressed sizes...) */
std::size_t headerBytes(const unsigned char *p, std::size_t available, const Format &format)
{
    if (!format.compressed)
        return format.headerSize;
    if (available < format.headerSize)
        return 0;
    const std::uint64_t blocks = readUInt(p, format.headerSize, format.bigEndian);
    return (3 + static_cast<std::size_t>(blocks)) * format.headerSize;
}

/**
 * Raw bytes of a binary array: @p data starts with the header, then the (compressed) data.
 * Returns false if the data is malformed.
 */
bool unpack(std::string_view data, const Format &format, std::string &out)
{
    const auto *p = reinterpret_cast<const unsigned char *>(data.data());
    const std::size_t h = format.headerSize;
    if (data.size() < h)
        return false;
    if (!format.compressed)
    {
        const std::uint64_t n = readUInt(p, h, format.bigEndian);
        if (data.size() < h + n)
            return false;
        out.assign(data.substr(h, static_cast<std::size_t>(n)));
        return true;
    }
    if (data.size() < 3 * h)
        return false;
    const auto blocks = static_cast<std::size_t>(readUInt(p, h, format.bigEndian));
    const auto blockSize = static_cast<std::size_t>(readUInt(p + h, h, format.bigEndian));
    auto lastSize = static_cast<std::size_t>(readUInt(p + 2 * h, h, format.bigEndian));
    if (lastSize == 0)
        lastSize = blockSize;
    const std::size_t header = (3 + blocks) * h;
    if (data.size() < header)
        return false;
    out.clear();
    std::size_t offset = header;
    for (std::size_t b = 0; b < blocks; ++b)
    {
        const auto compressed = static_cast<std::size_t>(readUInt(p + (3 + b) * h, h, format.bigEndian));
        if (offset + compressed > data.size())
            return false;
        const std::size_t size = b + 1 == blocks ? lastSize : blockSize;
        std::string block(size, '\0');
        mz_ulong length = static_cast<mz_ulong>(size);
        if (mz_uncompress(reinterpret_cast<unsigned char *>(block.data()), &length, p + offset,
                          static_cast<mz_ulong>(compressed)) != MZ_OK)
            return false;
        block.resize(length);
        out += block;
        offset += compressed;
    }
    return true;
}

/** Number of base64 characters that encode @p bytes bytes */
std::size_t encodedLength(std::size_t bytes)
{
    return 4 * ((bytes + 2) / 3);
}

/**
 * Binary data encoded in base64: VTK encodes the header and the data as separate blocks.
 * Returns the header followed by the data, decoded.
 */
bool decodeEncoded(std::string_view text, const Format &format, std::string &out)
{
    std::string clean;
    clean.reserve(text.size());
    for (char c : text)
        if (!std::isspace(static_cast<unsigned char>(c)))
            clean += c;
    // The first block holds the header; for compressed data its size is known after decoding
    // the number of blocks
    std::size_t first = encodedLength(format.headerSize);
    if (clean.size() < first)
        return false;
    std::string header = decodeBase64(std::string_view(clean).substr(0, first));
    if (format.compressed)
    {
        const std::size_t size = headerBytes(reinterpret_cast<const unsigned char *>(header.data()), header.size(), format);
        first = encodedLength(size);
        if (clean.size() < first)
            return false;
        header = decodeBase64(std::string_view(clean).substr(0, first));
    }
    // Only this array's data block (the text may continue with other arrays)
    const auto *hp = reinterpret_cast<const unsigned char *>(header.data());
    std::size_t dataBytes = 0;
    if (!format.compressed)
        dataBytes = static_cast<std::size_t>(readUInt(hp, format.headerSize, format.bigEndian));
    else
    {
        const auto blocks = static_cast<std::size_t>(readUInt(hp, format.headerSize, format.bigEndian));
        for (std::size_t b = 0; b < blocks; ++b)
            dataBytes += static_cast<std::size_t>(readUInt(hp + (3 + b) * format.headerSize, format.headerSize,
                                                           format.bigEndian));
    }
    out = header + decodeBase64(std::string_view(clean).substr(first, encodedLength(dataBytes)));
    return true;
}

std::size_t typeSize(const std::string &type)
{
    if (type == "Int8" || type == "UInt8")
        return 1;
    if (type == "Int16" || type == "UInt16")
        return 2;
    if (type == "Int32" || type == "UInt32" || type == "Float32")
        return 4;
    if (type == "Int64" || type == "UInt64" || type == "Float64")
        return 8;
    return 0;
}

void convert(const std::string &raw, const std::string &type, bool bigEndian, std::vector<double> &out)
{
    const std::size_t size = typeSize(type);
    const std::size_t n = size ? raw.size() / size : 0;
    out.resize(n);
    const bool isFloat = type.rfind("Float", 0) == 0;
    const bool isSigned = type.rfind("Int", 0) == 0;
    unsigned char b[8];
    for (std::size_t i = 0; i < n; ++i)
    {
        std::memcpy(b, raw.data() + i * size, size);
        if (bigEndian)
            std::reverse(b, b + size);
        if (isFloat && size == 4)
        {
            float f;
            std::memcpy(&f, b, 4);
            out[i] = f;
        }
        else if (isFloat)
            std::memcpy(&out[i], b, 8);
        else
        {
            std::uint64_t u = 0;
            for (int k = static_cast<int>(size) - 1; k >= 0; --k)
                u = (u << 8) | b[k];
            const int bits = static_cast<int>(8 * size);
            if (isSigned && bits < 64 && ((u >> (bits - 1)) & 1u))
                u |= ~std::uint64_t(0) << bits;
            out[i] = isSigned ? static_cast<double>(static_cast<std::int64_t>(u)) : static_cast<double>(u);
        }
    }
}

class XmlReader {
public:
    XmlReader(std::filesystem::path path, Format format, std::string appended, bool appendedBase64)
        : m_path(std::move(path)), m_format(format), m_appended(std::move(appended)), m_base64(appendedBase64)
    {
    }

    /** Values of a DataArray (all components, in order) */
    bool values(const pugi::xml_node &array, std::vector<double> &out) const
    {
        const std::string type = array.attribute("type").as_string();
        const std::string format = array.attribute("format").as_string("ascii");
        if (format == "ascii")
        {
            std::istringstream in(array.child_value());
            out.clear();
            std::string token;
            while (in >> token)
                out.push_back(std::strtod(token.c_str(), nullptr));
            return true;
        }
        if (typeSize(type) == 0)
            return false;
        std::string decoded, raw;
        std::string_view packed;
        if (format == "binary")
        {
            if (!decodeEncoded(array.child_value(), m_format, decoded))
                return false;
            packed = decoded;
        }
        else if (format == "appended")
        {
            const auto offset = static_cast<std::size_t>(array.attribute("offset").as_ullong());
            if (offset > m_appended.size())
                return false;
            if (m_base64)
            {
                // Base64 is decoded only up to this array's end, so stop at the next
                // appended array's text (decodeEncoded() reads the sizes from the header)
                if (!decodeEncoded(std::string_view(m_appended).substr(offset), m_format, decoded))
                    return false;
                packed = decoded;
            }
            else
                packed = std::string_view(m_appended).substr(offset);
        }
        else
            return false;
        if (!unpack(packed, m_format, raw))
            return false;
        convert(raw, type, m_format.bigEndian, out);
        return true;
    }

    std::unexpected<ivf::Error> fail(const std::string &why) const
    {
        return ivf::makeError(ivf::ErrorCode::InvalidData,
                              "Could not read VTK XML file '" + m_path.string() + "': " + why, Context);
    }

    /** Point arrays of a piece into @p set, starting at point @p first */
    bool pointData(const pugi::xml_node &piece, DataSet &set, std::size_t first, std::size_t count,
                   std::vector<std::pair<std::string, std::vector<double>>> &arrays,
                   std::vector<int> &components) const
    {
        std::vector<double> values;
        for (auto array : piece.child("PointData").children("DataArray"))
        {
            if (!this->values(array, values))
                return false;
            const std::string name = array.attribute("Name").as_string("values");
            const int c = std::max(1, array.attribute("NumberOfComponents").as_int(1));
            auto it = std::find_if(arrays.begin(), arrays.end(), [&](const auto &a) { return a.first == name; });
            if (it == arrays.end())
            {
                arrays.emplace_back(name, std::vector<double>(set.pointCount() * static_cast<std::size_t>(c), 0.0));
                components.push_back(c);
                it = arrays.end() - 1;
            }
            const std::size_t width = static_cast<std::size_t>(components[static_cast<std::size_t>(it - arrays.begin())]);
            std::copy_n(values.begin(), std::min(values.size(), count * width),
                        it->second.begin() + static_cast<std::ptrdiff_t>(first * width));
        }
        return true;
    }

private:
    std::filesystem::path m_path;
    Format m_format;
    std::string m_appended;
    bool m_base64;
};

} // namespace

ivf::Result<VtkData> ivfvis::detail::readVtkXml(const std::filesystem::path &path, const std::string &bytes)
{
    auto fail = [&](const std::string &why) {
        return ivf::makeError(ivf::ErrorCode::InvalidData,
                              "Could not read VTK XML file '" + path.string() + "': " + why, Context);
    };

    // Appended raw data is not XML: cut it out before parsing
    std::string xml = bytes, appended;
    bool appendedBase64 = false;
    if (const auto start = bytes.find("<AppendedData"); start != std::string::npos)
    {
        const auto tagEnd = bytes.find('>', start);
        const auto underscore = bytes.find('_', tagEnd);
        const auto end = bytes.rfind("</AppendedData>");
        if (tagEnd == std::string::npos || underscore == std::string::npos || end == std::string::npos || end < underscore)
            return fail("malformed AppendedData");
        const std::string tag = bytes.substr(start, tagEnd - start);
        appendedBase64 = tag.find("\"base64\"") != std::string::npos;
        appended = bytes.substr(underscore + 1, end - underscore - 1);
        xml = bytes.substr(0, start) + "</VTKFile>";
    }

    pugi::xml_document doc;
    const pugi::xml_parse_result parsed = doc.load_buffer(xml.data(), xml.size());
    if (!parsed)
        return fail(std::string("not valid XML (") + parsed.description() + ")");
    const pugi::xml_node root = doc.child("VTKFile");
    if (!root)
        return ivf::makeError(ivf::ErrorCode::UnsupportedFormat,
                              "Could not read VTK XML file '" + path.string() + "': no VTKFile element", Context);

    Format format;
    format.bigEndian = std::string(root.attribute("byte_order").as_string("LittleEndian")) == "BigEndian";
    format.headerSize = std::string(root.attribute("header_type").as_string("UInt32")) == "UInt64" ? 8 : 4;
    format.compressed = std::string(root.attribute("compressor").as_string()).find("ZLib") != std::string::npos;
    if (!root.attribute("compressor").empty() && !format.compressed &&
        std::string(root.attribute("compressor").as_string()).size() > 0)
        return ivf::makeError(ivf::ErrorCode::UnsupportedFormat,
                              "Could not read VTK XML file '" + path.string() + "': compressor '" +
                                  root.attribute("compressor").as_string() + "' is not supported (zlib is)",
                              Context);
    const XmlReader reader(path, format, std::move(appended), appendedBase64);
    const std::string type = root.attribute("type").as_string();
    VtkData data;
    std::vector<double> values;

    if (type == "ImageData")
    {
        const pugi::xml_node image = root.child("ImageData");
        int whole[6] = {0, 0, 0, 0, 0, 0};
        std::istringstream(image.attribute("WholeExtent").as_string()) >> whole[0] >> whole[1] >> whole[2] >> whole[3] >>
            whole[4] >> whole[5];
        glm::vec3 origin(0.0f), spacing(1.0f);
        std::istringstream(image.attribute("Origin").as_string("0 0 0")) >> origin.x >> origin.y >> origin.z;
        std::istringstream(image.attribute("Spacing").as_string("1 1 1")) >> spacing.x >> spacing.y >> spacing.z;
        const glm::ivec3 dims(whole[1] - whole[0] + 1, whole[3] - whole[2] + 1, whole[5] - whole[4] + 1);
        data.grid = UniformGrid::create(dims, origin + glm::vec3(whole[0], whole[2], whole[4]) * spacing, spacing);

        std::vector<std::pair<std::string, std::vector<double>>> arrays;
        std::vector<int> components;
        for (auto piece : image.children("Piece"))
        {
            int e[6] = {0, 0, 0, 0, 0, 0};
            std::istringstream(piece.attribute("Extent").as_string()) >> e[0] >> e[1] >> e[2] >> e[3] >> e[4] >> e[5];
            const glm::ivec3 pd(e[1] - e[0] + 1, e[3] - e[2] + 1, e[5] - e[4] + 1);
            for (auto array : piece.child("PointData").children("DataArray"))
            {
                if (!reader.values(array, values))
                    return reader.fail("cannot decode array '" + std::string(array.attribute("Name").as_string()) + "'");
                const std::string name = array.attribute("Name").as_string("values");
                const int c = std::max(1, array.attribute("NumberOfComponents").as_int(1));
                auto it = std::find_if(arrays.begin(), arrays.end(), [&](const auto &a) { return a.first == name; });
                if (it == arrays.end())
                {
                    arrays.emplace_back(name, std::vector<double>(data.grid->pointCount() * static_cast<std::size_t>(c)));
                    components.push_back(c);
                    it = arrays.end() - 1;
                }
                // Place the piece's points by its extent
                std::size_t src = 0;
                for (int k = 0; k < pd.z; ++k)
                    for (int j = 0; j < pd.y; ++j)
                        for (int i = 0; i < pd.x; ++i, ++src)
                        {
                            const std::size_t dst = data.grid->index(e[0] - whole[0] + i, e[2] - whole[2] + j,
                                                                     e[4] - whole[4] + k);
                            for (int q = 0; q < c; ++q)
                                if (src * static_cast<std::size_t>(c) + static_cast<std::size_t>(q) < values.size())
                                    it->second[dst * static_cast<std::size_t>(c) + static_cast<std::size_t>(q)] =
                                        values[src * static_cast<std::size_t>(c) + static_cast<std::size_t>(q)];
                        }
            }
        }
        for (std::size_t a = 0; a < arrays.size(); ++a)
            addPointArray(*data.grid, arrays[a].first, arrays[a].second, components[a]);
        return data;
    }

    if (type != "UnstructuredGrid" && type != "PolyData")
        return ivf::makeError(ivf::ErrorCode::UnsupportedFormat,
                              "Could not read VTK XML file '" + path.string() + "': data set type '" + type +
                                  "' is not supported (UnstructuredGrid, PolyData and ImageData are)",
                              Context);

    // Points of all pieces first, then their cells and arrays
    const pugi::xml_node set = root.child(type.c_str());
    std::vector<glm::vec3> positions;
    std::vector<std::size_t> firstPoint;
    for (auto piece : set.children("Piece"))
    {
        firstPoint.push_back(positions.size());
        const auto n = static_cast<std::size_t>(piece.attribute("NumberOfPoints").as_ullong());
        const pugi::xml_node array = piece.child("Points").child("DataArray");
        if (!reader.values(array, values) || values.size() < 3 * n)
            return reader.fail("cannot decode the points");
        for (std::size_t i = 0; i < n; ++i)
            positions.emplace_back(static_cast<float>(values[3 * i]), static_cast<float>(values[3 * i + 1]),
                                   static_cast<float>(values[3 * i + 2]));
    }
    data.points = PointSet::create(std::move(positions));

    CellCollector collector(data);
    std::vector<std::pair<std::string, std::vector<double>>> arrays;
    std::vector<int> components;
    std::size_t pieceIndex = 0;
    for (auto piece : set.children("Piece"))
    {
        const std::size_t first = firstPoint[pieceIndex++];
        const auto n = static_cast<std::size_t>(piece.attribute("NumberOfPoints").as_ullong());

        auto cellArray = [&](const pugi::xml_node &cells, const char *name, std::vector<double> &out) {
            for (auto array : cells.children("DataArray"))
                if (std::string(array.attribute("Name").as_string()) == name)
                    return reader.values(array, out);
            out.clear();
            return true;
        };
        auto addCells = [&](const pugi::xml_node &cells, int fixedType) {
            std::vector<double> connectivity, offsets, types;
            if (!cellArray(cells, "connectivity", connectivity) || !cellArray(cells, "offsets", offsets) ||
                (fixedType == 0 && !cellArray(cells, "types", types)))
                return false;
            std::vector<std::uint32_t> ids;
            std::size_t start = 0;
            for (std::size_t c = 0; c < offsets.size(); ++c)
            {
                const auto end = static_cast<std::size_t>(offsets[c]);
                ids.clear();
                for (std::size_t k = start; k < end && k < connectivity.size(); ++k)
                    ids.push_back(static_cast<std::uint32_t>(first + static_cast<std::size_t>(connectivity[k])));
                collector.add(fixedType != 0 ? fixedType : static_cast<int>(c < types.size() ? types[c] : 0), ids);
                start = end;
            }
            return true;
        };
        bool ok = true;
        if (type == "UnstructuredGrid")
            ok = addCells(piece.child("Cells"), 0);
        else
            ok = addCells(piece.child("Lines"), VtkPolyLine) && addCells(piece.child("Strips"), VtkTriangleStrip) &&
                 addCells(piece.child("Polys"), VtkPolygon);
        if (!ok)
            return reader.fail("cannot decode the cells");
        if (!reader.pointData(piece, *data.points, first, n, arrays, components))
            return reader.fail("cannot decode the point data");
    }
    for (std::size_t a = 0; a < arrays.size(); ++a)
        addPointArray(*data.points, arrays[a].first, arrays[a].second, components[a]);
    if (const int skipped = collector.finish(); skipped > 0)
        ivf::logWarning(ivf::logStr(skipped, " cells of unsupported types in '", path.string(), "' left out"), Context);
    return data;
}

#endif // IVFVIS_WITH_VTK_XML
