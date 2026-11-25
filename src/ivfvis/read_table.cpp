// CSV and NumPy readers

#include <ivfvis/readers.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>

using namespace ivfvis;

namespace {

constexpr const char *Context = "Readers";
constexpr float NaN = std::numeric_limits<float>::quiet_NaN();

ivf::Result<std::string> readFile(const std::filesystem::path &path, std::string_view what)
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec))
        return ivf::makeError(ivf::ErrorCode::FileNotFound,
                              "Could not read " + std::string(what) + " '" + path.string() + "': file not found",
                              Context);
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return ivf::makeError(ivf::ErrorCode::ReadFailed,
                              "Could not read " + std::string(what) + " '" + path.string() + "': cannot open it",
                              Context);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::string trim(std::string_view s)
{
    std::size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a])))
        ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1])))
        --b;
    return std::string(s.substr(a, b - a));
}

/** The number in @p field, or NaN */
float number(const std::string &field)
{
    if (field.empty())
        return NaN;
    char *end = nullptr;
    const float value = std::strtof(field.c_str(), &end);
    return end == field.c_str() + field.size() ? value : NaN;
}

std::vector<std::string> split(const std::string &line, char delimiter)
{
    std::vector<std::string> fields;
    std::string field;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i)
    {
        const char c = line[i];
        if (quoted)
        {
            if (c == '"' && i + 1 < line.size() && line[i + 1] == '"')
            {
                field += '"';
                ++i;
            }
            else if (c == '"')
                quoted = false;
            else
                field += c;
        }
        else if (c == '"')
            quoted = true;
        else if (delimiter == ' ' ? std::isspace(static_cast<unsigned char>(c)) != 0 : c == delimiter)
        {
            if (delimiter != ' ' || !field.empty())
                fields.push_back(trim(field));
            field.clear();
        }
        else
            field += c;
    }
    if (delimiter != ' ' || !field.empty())
        fields.push_back(trim(field));
    return fields;
}

} // namespace

// ---------------------------------------------------------------------------------------
// CSV
// ---------------------------------------------------------------------------------------

std::size_t Table::rowCount() const
{
    return columns.empty() ? 0 : columns.front().size();
}

int Table::column(std::string_view name) const
{
    for (std::size_t i = 0; i < names.size(); ++i)
        if (names[i] == name)
            return static_cast<int>(i);
    return -1;
}

std::span<const float> Table::values(std::string_view name) const
{
    const int c = column(name);
    return c < 0 ? std::span<const float>() : std::span<const float>(columns[static_cast<std::size_t>(c)]);
}

ivf::Result<Table> ivfvis::readCsv(const std::filesystem::path &path, const CsvOptions &options)
{
    auto text = readFile(path, "CSV file");
    if (!text)
        return std::unexpected(text.error());

    std::vector<std::string> lines;
    {
        std::istringstream in(*text);
        std::string line;
        while (std::getline(in, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            const std::string t = trim(line);
            if (t.empty() || (options.comment != 0 && t.front() == options.comment))
                continue;
            lines.push_back(line);
        }
    }
    if (lines.empty())
        return ivf::makeError(ivf::ErrorCode::InvalidData, "Could not read CSV file '" + path.string() + "': no data",
                              Context);

    char delimiter = options.delimiter;
    if (delimiter == 0)
    {
        const std::string &probe = lines.size() > 1 ? lines[1] : lines[0];
        std::size_t best = 0;
        delimiter = ' ';
        for (char d : {',', ';', '\t'})
        {
            const auto n = static_cast<std::size_t>(std::count(probe.begin(), probe.end(), d));
            if (n > best)
            {
                best = n;
                delimiter = d;
            }
        }
    }

    Table table;
    std::size_t first = 0;
    const std::vector<std::string> firstRow = split(lines[0], delimiter);
    bool header = options.header == 1;
    if (options.header < 0)
        header = std::any_of(firstRow.begin(), firstRow.end(),
                             [](const std::string &f) { return !f.empty() && std::isnan(number(f)); });
    if (header)
    {
        table.names = firstRow;
        first = 1;
    }

    for (std::size_t r = first; r < lines.size(); ++r)
    {
        const auto fields = split(lines[r], delimiter);
        if (fields.size() > table.columns.size())
            table.columns.resize(fields.size(), std::vector<float>(r - first, NaN));
        for (std::size_t c = 0; c < table.columns.size(); ++c)
            table.columns[c].push_back(c < fields.size() ? number(fields[c]) : NaN);
    }
    table.names.resize(table.columns.size());
    for (std::size_t c = 0; c < table.names.size(); ++c)
        if (table.names[c].empty())
            table.names[c] = "column" + std::to_string(c + 1);
    return table;
}

ivf::Result<PointSetPtr> ivfvis::pointsFromTable(const Table &table, std::string_view x, std::string_view y,
                                                 std::string_view z)
{
    const int cx = x.empty() ? -1 : table.column(x), cy = y.empty() ? -1 : table.column(y),
              cz = z.empty() ? -1 : table.column(z);
    for (auto [name, index] : {std::pair{x, cx}, std::pair{y, cy}, std::pair{z, cz}})
        if (!name.empty() && index < 0)
            return ivf::makeError(ivf::ErrorCode::InvalidArgument, "No column '" + std::string(name) + "' in the table",
                                  Context);

    const std::size_t n = table.rowCount();
    std::vector<glm::vec3> positions(n, glm::vec3(0.0f));
    for (std::size_t i = 0; i < n; ++i)
        for (int a = 0; a < 3; ++a)
        {
            const int c = a == 0 ? cx : (a == 1 ? cy : cz);
            if (c >= 0)
                positions[i][a] = table.columns[static_cast<std::size_t>(c)][i];
        }
    auto points = PointSet::create(std::move(positions));
    for (std::size_t c = 0; c < table.columns.size(); ++c)
    {
        const auto ci = static_cast<int>(c);
        const auto &column = table.columns[c];
        if (ci == cx || ci == cy || ci == cz || std::all_of(column.begin(), column.end(), [](float v) { return std::isnan(v); }))
            continue;
        std::copy(column.begin(), column.end(), points->addScalars(table.names[c]).begin());
    }
    return points;
}

// ---------------------------------------------------------------------------------------
// NumPy
// ---------------------------------------------------------------------------------------

std::size_t NpyArray::size() const
{
    std::size_t n = 1;
    for (std::size_t s : shape)
        n *= s;
    return n;
}

ivf::Result<NpyArray> ivfvis::readNpy(const std::filesystem::path &path)
{
    auto bytes = readFile(path, "NumPy file");
    if (!bytes)
        return std::unexpected(bytes.error());
    const std::string &b = *bytes;
    auto fail = [&](const std::string &why) {
        return ivf::makeError(ivf::ErrorCode::InvalidData, "Could not read NumPy file '" + path.string() + "': " + why,
                              Context);
    };
    if (b.size() < 10 || b.compare(0, 6, "\x93NUMPY") != 0)
        return ivf::makeError(ivf::ErrorCode::UnsupportedFormat,
                              "Could not read NumPy file '" + path.string() + "': not a .npy file", Context);
    const int major = static_cast<unsigned char>(b[6]);
    std::size_t headerLength = 0, headerStart = 0;
    auto byte = [&](std::size_t i) { return static_cast<std::size_t>(static_cast<unsigned char>(b[i])); };
    if (major == 1)
    {
        headerLength = byte(8) | (byte(9) << 8);
        headerStart = 10;
    }
    else if (major == 2 || major == 3)
    {
        if (b.size() < 12)
            return fail("truncated header");
        headerLength = byte(8) | (byte(9) << 8) | (byte(10) << 16) | (byte(11) << 24);
        headerStart = 12;
    }
    else
        return fail("unknown version " + std::to_string(major));
    if (b.size() < headerStart + headerLength)
        return fail("truncated header");
    const std::string header = b.substr(headerStart, headerLength);

    // Header dictionary: {'descr': '<f4', 'fortran_order': False, 'shape': (2, 3), }
    auto value = [&](const std::string &key) -> std::string {
        const auto k = header.find("'" + key + "'");
        if (k == std::string::npos)
            return {};
        const auto colon = header.find(':', k);
        return colon == std::string::npos ? std::string() : header.substr(colon + 1);
    };
    NpyArray array;
    {
        const std::string d = value("descr");
        const auto q1 = d.find('\''), q2 = d.find('\'', q1 + 1);
        if (q1 == std::string::npos || q2 == std::string::npos)
            return fail("no data type");
        array.dtype = d.substr(q1 + 1, q2 - q1 - 1);
    }
    array.fortranOrder = trim(value("fortran_order")).rfind("True", 0) == 0;
    {
        const std::string s = value("shape");
        const auto p1 = s.find('('), p2 = s.find(')');
        if (p1 == std::string::npos || p2 == std::string::npos)
            return fail("no shape");
        std::istringstream in(s.substr(p1 + 1, p2 - p1 - 1));
        std::string item;
        while (std::getline(in, item, ','))
        {
            const std::string t = trim(item);
            if (!t.empty())
                array.shape.push_back(static_cast<std::size_t>(std::stoull(t)));
        }
    }

    // Type: byte order, kind, size
    if (array.dtype.size() < 3)
        return fail("unsupported data type '" + array.dtype + "'");
    const char order = array.dtype[0], kind = array.dtype[1];
    const int size = std::atoi(array.dtype.c_str() + 2);
    const bool swap = order == '>';
    const bool known = (kind == 'f' && (size == 4 || size == 8)) || ((kind == 'i' || kind == 'u') &&
                       (size == 1 || size == 2 || size == 4 || size == 8)) || (kind == 'b' && size == 1);
    if (!known)
        return ivf::makeError(ivf::ErrorCode::UnsupportedFormat,
                              "Could not read NumPy file '" + path.string() + "': data type '" + array.dtype +
                                  "' is not supported",
                              Context);

    const std::size_t count = array.size();
    const std::size_t dataStart = headerStart + headerLength;
    if (b.size() < dataStart + count * static_cast<std::size_t>(size))
        return fail("fewer values than the shape needs");
    array.data.resize(count);
    unsigned char raw[8];
    for (std::size_t i = 0; i < count; ++i)
    {
        std::memcpy(raw, b.data() + dataStart + i * static_cast<std::size_t>(size), static_cast<std::size_t>(size));
        if (swap)
            std::reverse(raw, raw + size);
        double v = 0.0;
        switch (kind)
        {
        case 'f':
            if (size == 4)
            {
                float f;
                std::memcpy(&f, raw, 4);
                v = f;
            }
            else
                std::memcpy(&v, raw, 8);
            break;
        case 'b':
            v = raw[0] != 0 ? 1.0 : 0.0;
            break;
        case 'u':
        {
            std::uint64_t u = 0;
            for (int k = size - 1; k >= 0; --k)
                u = (u << 8) | raw[k];
            v = static_cast<double>(u);
            break;
        }
        default: // 'i'
        {
            std::uint64_t u = 0;
            for (int k = size - 1; k >= 0; --k)
                u = (u << 8) | raw[k];
            const int bits = 8 * size;
            if (bits < 64 && (u >> (bits - 1)) & 1u)
                u |= ~std::uint64_t(0) << bits; // sign extension
            v = static_cast<double>(static_cast<std::int64_t>(u));
            break;
        }
        }
        array.data[i] = static_cast<float>(v);
    }
    return array;
}

ivf::Result<UniformGridPtr> ivfvis::gridFromNpy(const NpyArray &array, std::string_view name, const glm::vec3 &origin,
                                                const glm::vec3 &spacing)
{
    const std::size_t rank = array.shape.size();
    const bool vectors = rank == 4 && (array.fortranOrder ? array.shape[3] == 3 : array.shape[3] == 3);
    if (rank < 1 || rank > 4 || (rank == 4 && !vectors))
        return ivf::makeError(ivf::ErrorCode::InvalidArgument,
                              "A grid needs a 1D, 2D or 3D array, or a 3D array of 3-vectors", Context);
    const std::size_t spatial = vectors ? 3 : rank;
    glm::ivec3 dims(1);
    for (std::size_t a = 0; a < spatial; ++a)
    {
        // C order: the last spatial index varies fastest (x); Fortran order: the first
        const std::size_t s = array.fortranOrder ? array.shape[a] : array.shape[spatial - 1 - a];
        dims[static_cast<int>(a)] = static_cast<int>(s);
    }
    auto grid = UniformGrid::create(dims, origin, spacing);
    const std::size_t n = grid->pointCount();
    if (!vectors)
    {
        std::copy(array.data.begin(), array.data.begin() + static_cast<std::ptrdiff_t>(n),
                  grid->addScalars(name).begin());
        return grid;
    }
    auto v = grid->addVectors(name);
    for (std::size_t i = 0; i < n; ++i)
        for (int c = 0; c < 3; ++c)
            v[i][c] = array.fortranOrder ? array.data[i + n * static_cast<std::size_t>(c)]
                                         : array.data[3 * i + static_cast<std::size_t>(c)];
    return grid;
}

ivf::Result<PointSetPtr> ivfvis::pointsFromNpy(const NpyArray &array)
{
    if (array.shape.size() != 2 || array.shape[1] < 2)
        return ivf::makeError(ivf::ErrorCode::InvalidArgument, "Points need an array of shape (n, 2) or (n, k >= 3)",
                              Context);
    const std::size_t n = array.shape[0], k = array.shape[1];
    auto at = [&](std::size_t i, std::size_t c) {
        return array.fortranOrder ? array.data[i + n * c] : array.data[i * k + c];
    };
    std::vector<glm::vec3> positions(n, glm::vec3(0.0f));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t c = 0; c < std::min<std::size_t>(k, 3); ++c)
            positions[i][static_cast<int>(c)] = at(i, c);
    auto points = PointSet::create(std::move(positions));
    for (std::size_t c = 3; c < k; ++c)
    {
        auto s = points->addScalars("column" + std::to_string(c));
        for (std::size_t i = 0; i < n; ++i)
            s[i] = at(i, c);
    }
    return points;
}
