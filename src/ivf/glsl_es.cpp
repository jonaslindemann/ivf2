#include <ivf/glsl_es.h>

#include <ivf/logger.h>
#include <ivf/render_context.h>

#include <glad/glad.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <unordered_map>

using namespace ivf;

namespace {

// Default precisions: ES has none for floats in fragment shaders and none for most sampler
// types; high precision everywhere matches desktop GL (float render targets, depth maps).
constexpr std::string_view EsHeader = "#version 300 es\n"
                                      "precision highp float;\n"
                                      "precision highp int;\n"
                                      "precision highp sampler2D;\n"
                                      "precision highp sampler3D;\n"
                                      "precision highp samplerCube;\n"
                                      "precision highp sampler2DArray;\n"
                                      "precision highp sampler2DShadow;\n"
                                      "precision highp samplerCubeShadow;\n"
                                      "precision highp sampler2DArrayShadow;\n"
                                      "precision highp isampler2D;\n"
                                      "precision highp isampler3D;\n"
                                      "precision highp usampler2D;\n"
                                      "precision highp usampler3D;\n";

bool isIdentChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool isSpace(char c)
{
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

std::string trim(std::string_view s)
{
    std::size_t b = 0, e = s.size();
    while (b < e && isSpace(s[b]))
        ++b;
    while (e > b && isSpace(s[e - 1]))
        --e;
    return std::string(s.substr(b, e - b));
}

// Copy of the source with comments blanked out (newlines kept), so searches skip them and
// positions match the original.
std::string maskComments(std::string_view s)
{
    std::string out(s);
    std::size_t i = 0;
    while (i < s.size())
    {
        if (s[i] == '/' && i + 1 < s.size() && s[i + 1] == '/')
        {
            while (i < s.size() && s[i] != '\n')
                out[i++] = ' ';
        }
        else if (s[i] == '/' && i + 1 < s.size() && s[i + 1] == '*')
        {
            out[i] = out[i + 1] = ' ';
            i += 2;
            while (i < s.size() && !(s[i] == '*' && i + 1 < s.size() && s[i + 1] == '/'))
            {
                if (s[i] != '\n')
                    out[i] = ' ';
                ++i;
            }
            if (i < s.size())
            {
                out[i] = out[i + 1] = ' ';
                i += 2;
            }
        }
        else
            ++i;
    }
    return out;
}

// Object-like macros (#define NAME value), used to evaluate initialisers such as
// `uniform int blendMode = TEX_BLEND_MULTIPLY;`.
std::unordered_map<std::string, std::string> collectDefines(const std::string &masked)
{
    std::unordered_map<std::string, std::string> defines;
    std::size_t lineStart = 0;
    while (lineStart < masked.size())
    {
        std::size_t lineEnd = masked.find('\n', lineStart);
        if (lineEnd == std::string::npos)
            lineEnd = masked.size();
        const std::string line = trim(std::string_view(masked).substr(lineStart, lineEnd - lineStart));
        if (line.size() > 1 && line[0] == '#')
        {
            const std::string directive = trim(std::string_view(line).substr(1));
            if (directive.rfind("define", 0) == 0 && directive.size() > 6 && isSpace(directive[6]))
            {
                const std::string rest = trim(std::string_view(directive).substr(6));
                std::size_t n = 0;
                while (n < rest.size() && isIdentChar(rest[n]))
                    ++n;
                if (n > 0 && (n == rest.size() || rest[n] != '(')) // not a function-like macro
                    defines[rest.substr(0, n)] = trim(std::string_view(rest).substr(n));
            }
        }
        lineStart = lineEnd + 1;
    }
    return defines;
}

// Number of components of a scalar or vector type; 0 for types without default support.
int componentCount(std::string_view type)
{
    if (type == "float" || type == "int" || type == "uint" || type == "bool")
        return 1;
    for (std::string_view prefix : {"vec", "ivec", "uvec", "bvec"})
        if (type.size() == prefix.size() + 1 && type.substr(0, prefix.size()) == prefix)
        {
            const char n = type.back();
            if (n >= '2' && n <= '4')
                return n - '0';
        }
    return 0;
}

bool isIntegerType(std::string_view type)
{
    return type == "int" || type == "uint" || type == "bool" || type.starts_with("ivec") || type.starts_with("uvec") ||
           type.starts_with("bvec");
}

// Evaluates constant initialisers: numbers, true/false, macros, (unary) signs, parentheses,
// and scalar, vector and array constructors. Results are flattened to their components.
class ConstantEvaluator {
public:
    explicit ConstantEvaluator(const std::unordered_map<std::string, std::string> &defines) : m_defines(defines)
    {}

    bool evaluate(std::string_view text, std::vector<double> &values, int *elements = nullptr, int depth = 0)
    {
        if (depth > 8)
            return false;
        Parser p{text, 0};
        int count = 0;
        if (!expression(p, values, &count, depth))
            return false;
        p.skipSpace();
        if (p.pos != p.text.size())
            return false;
        if (elements != nullptr)
            *elements = count;
        return true;
    }

private:
    struct Parser {
        std::string_view text;
        std::size_t pos;

        void skipSpace()
        {
            while (pos < text.size() && isSpace(text[pos]))
                ++pos;
        }
        char peek()
        {
            skipSpace();
            return pos < text.size() ? text[pos] : '\0';
        }
        std::string identifier()
        {
            skipSpace();
            const std::size_t start = pos;
            if (pos < text.size() && (std::isalpha(static_cast<unsigned char>(text[pos])) || text[pos] == '_'))
                while (pos < text.size() && isIdentChar(text[pos]))
                    ++pos;
            return std::string(text.substr(start, pos - start));
        }
    };

    // a op b, component-wise; a scalar operand applies to every component of the other
    static bool combine(std::vector<double> &a, const std::vector<double> &b, char op)
    {
        if (a.size() != b.size() && a.size() != 1 && b.size() != 1)
            return false;
        std::vector<double> r(std::max(a.size(), b.size()));
        for (std::size_t i = 0; i < r.size(); ++i)
        {
            const double x = a[a.size() == 1 ? 0 : i], y = b[b.size() == 1 ? 0 : i];
            r[i] = op == '+' ? x + y : op == '-' ? x - y : op == '*' ? x * y : x / y;
        }
        a = std::move(r);
        return true;
    }

    // Sums and differences of terms. elements: number of array elements for array
    // constructors, 1 otherwise.
    bool expression(Parser &p, std::vector<double> &out, int *elements, int depth)
    {
        std::vector<double> value;
        if (!term(p, value, elements, depth))
            return false;
        while (p.peek() == '+' || p.peek() == '-')
        {
            const char op = p.text[p.pos++];
            std::vector<double> rhs;
            if (!term(p, rhs, nullptr, depth) || !combine(value, rhs, op))
                return false;
        }
        out.insert(out.end(), value.begin(), value.end());
        return true;
    }

    // Products and quotients of signed values
    bool term(Parser &p, std::vector<double> &out, int *elements, int depth)
    {
        std::vector<double> value;
        if (!unary(p, value, elements, depth))
            return false;
        while (p.peek() == '*' || p.peek() == '/')
        {
            const char op = p.text[p.pos++];
            std::vector<double> rhs;
            if (!unary(p, rhs, nullptr, depth) || !combine(value, rhs, op))
                return false;
        }
        out.insert(out.end(), value.begin(), value.end());
        return true;
    }

    bool unary(Parser &p, std::vector<double> &out, int *elements, int depth)
    {
        if (elements != nullptr)
            *elements = 1;
        double sign = 1.0;
        while (p.peek() == '-' || p.peek() == '+')
        {
            if (p.text[p.pos] == '-')
                sign = -sign;
            ++p.pos;
        }

        std::vector<double> values;
        const char c = p.peek();
        if (c == '(')
        {
            ++p.pos;
            if (!expression(p, values, nullptr, depth) || p.peek() != ')')
                return false;
            ++p.pos;
        }
        else if (std::isdigit(static_cast<unsigned char>(c)) || c == '.')
        {
            const char *begin = p.text.data() + p.pos;
            char *end = nullptr;
            const bool hex = p.text.substr(p.pos).starts_with("0x") || p.text.substr(p.pos).starts_with("0X");
            const double value = hex ? static_cast<double>(std::strtoull(begin, &end, 16)) : std::strtod(begin, &end);
            if (end == begin)
                return false;
            p.pos += static_cast<std::size_t>(end - begin);
            while (p.pos < p.text.size() && (p.text[p.pos] == 'f' || p.text[p.pos] == 'F' || p.text[p.pos] == 'u' ||
                                             p.text[p.pos] == 'U'))
                ++p.pos;
            values.push_back(value);
        }
        else
        {
            const std::string name = p.identifier();
            if (name.empty())
                return false;
            if (name == "true" || name == "false")
                values.push_back(name == "true" ? 1.0 : 0.0);
            else if (p.peek() == '(' || p.peek() == '[')
            {
                if (!construct(p, name, values, elements, depth))
                    return false;
            }
            else if (auto it = m_defines.find(name); it != m_defines.end())
            {
                if (!evaluate(it->second, values, elements, depth + 1))
                    return false;
            }
            else
                return false;
        }

        for (double &v : values)
            v *= sign;
        out.insert(out.end(), values.begin(), values.end());
        return true;
    }

    // type(args) or type[n](args) / type[](args)
    bool construct(Parser &p, const std::string &type, std::vector<double> &out, int *elements, int depth)
    {
        bool array = false;
        if (p.peek() == '[')
        {
            array = true;
            const std::size_t close = p.text.find(']', p.pos);
            if (close == std::string_view::npos)
                return false;
            p.pos = close + 1;
        }
        if (p.peek() != '(')
            return false;
        ++p.pos;

        std::vector<std::vector<double>> args;
        if (p.peek() != ')')
        {
            while (true)
            {
                std::vector<double> arg;
                if (!expression(p, arg, nullptr, depth))
                    return false;
                args.push_back(std::move(arg));
                if (p.peek() == ',')
                {
                    ++p.pos;
                    continue;
                }
                break;
            }
        }
        if (p.peek() != ')')
            return false;
        ++p.pos;

        const int n = componentCount(type);
        if (n == 0)
            return false;
        if (array)
        {
            for (const auto &arg : args)
            {
                if (static_cast<int>(arg.size()) != n)
                    return false;
                out.insert(out.end(), arg.begin(), arg.end());
            }
            if (elements != nullptr)
                *elements = static_cast<int>(args.size());
            return true;
        }

        std::vector<double> components;
        for (const auto &arg : args)
            components.insert(components.end(), arg.begin(), arg.end());
        if (components.size() == 1 && n > 1)
        {
            const double scalar = components[0]; // not a reference into the vector being assigned
            components.assign(static_cast<std::size_t>(n), scalar);
        }
        if (static_cast<int>(components.size()) < n)
            return false;
        components.resize(static_cast<std::size_t>(n)); // float(x) from a vector takes the first component
        out.insert(out.end(), components.begin(), components.end());
        return true;
    }

    const std::unordered_map<std::string, std::string> &m_defines;
};

} // namespace

GlslEsTranslation ivf::translateToGlslEs(std::string_view source)
{
    GlslEsTranslation result;
    std::string src(source);
    const std::string masked = maskComments(src);
    const auto defines = collectDefines(masked);
    ConstantEvaluator evaluator(defines);

    // 1. Uniform initialisers: `uniform <type> <name>[<n>] = <value>;` loses ` = <value>`.
    struct Removal {
        std::size_t begin, end;
    };
    std::vector<Removal> removals;
    std::size_t pos = 0;
    while ((pos = masked.find("uniform", pos)) != std::string::npos)
    {
        const std::size_t start = pos;
        pos += 7;
        if ((start > 0 && isIdentChar(masked[start - 1])) || (pos < masked.size() && isIdentChar(masked[pos])))
            continue;

        std::size_t p = pos;
        auto skipSpace = [&] {
            while (p < masked.size() && isSpace(masked[p]))
                ++p;
        };
        auto identifier = [&] {
            skipSpace();
            const std::size_t b = p;
            while (p < masked.size() && isIdentChar(masked[p]))
                ++p;
            return masked.substr(b, p - b);
        };

        std::string type = identifier();
        if (type == "highp" || type == "mediump" || type == "lowp")
            type = identifier();
        const std::string name = identifier();
        if (type.empty() || name.empty())
            continue; // e.g. a uniform block: uniform IvfCamera { ... }
        std::size_t declarationEnd = p; // what follows is removed with the initialiser
        skipSpace();

        bool isArray = false;
        std::string sizeText;
        if (p < masked.size() && masked[p] == '[')
        {
            const std::size_t close = masked.find(']', p);
            if (close == std::string::npos)
                continue;
            isArray = true;
            sizeText = trim(std::string_view(masked).substr(p + 1, close - p - 1));
            p = close + 1;
            declarationEnd = p;
            skipSpace();
        }
        if (p >= masked.size() || masked[p] != '=')
            continue;

        // The initialiser ends at the first ';' outside brackets
        const std::size_t equals = p;
        int depth = 0;
        std::size_t q = equals + 1;
        for (; q < masked.size(); ++q)
        {
            const char c = masked[q];
            if (c == '(' || c == '[')
                ++depth;
            else if (c == ')' || c == ']')
                --depth;
            else if (c == ';' && depth == 0)
                break;
        }
        if (q >= masked.size())
            break;
        removals.push_back({declarationEnd, q});
        pos = q;

        UniformDefault value;
        value.type = type;
        value.name = name;
        const std::string expression = trim(std::string_view(masked).substr(equals + 1, q - equals - 1));
        int elements = 1;
        bool ok = componentCount(type) > 0 && evaluator.evaluate(expression, value.values, &elements);
        if (ok && isArray)
        {
            value.arraySize = elements;
            if (!sizeText.empty())
            {
                std::vector<double> size;
                ok = evaluator.evaluate(sizeText, size) && size.size() == 1 && static_cast<int>(size[0]) == elements;
            }
        }
        ok = ok && static_cast<int>(value.values.size()) == componentCount(type) * std::max(1, value.arraySize);
        if (ok)
            result.defaults.push_back(std::move(value));
        else
            result.warnings.push_back("initial value of uniform '" + name + "' not supported: " + expression);
    }

    // Remove them back to front (positions stay valid); keep the newlines for line numbers
    for (auto it = removals.rbegin(); it != removals.rend(); ++it)
    {
        const std::string newlines(static_cast<std::size_t>(std::count(src.begin() + static_cast<std::ptrdiff_t>(it->begin),
                                                                         src.begin() + static_cast<std::ptrdiff_t>(it->end), '\n')),
                                   '\n');
        src.replace(it->begin, it->end - it->begin, newlines);
    }

    // 2. The #version line (before all initialisers, so its position is unchanged)
    std::size_t versionPos = std::string::npos;
    for (std::size_t lineStart = 0; lineStart < masked.size();)
    {
        std::size_t first = lineStart;
        while (first < masked.size() && masked[first] != '\n' && isSpace(masked[first]))
            ++first;
        if (masked.compare(first, 8, "#version") == 0)
        {
            versionPos = lineStart;
            break;
        }
        const std::size_t next = masked.find('\n', lineStart);
        if (next == std::string::npos)
            break;
        lineStart = next + 1;
    }

    std::string header(EsHeader);
    if (versionPos == std::string::npos)
        result.source = header + "#line 1\n" + src;
    else
    {
        std::size_t lineEnd = src.find('\n', versionPos);
        lineEnd = lineEnd == std::string::npos ? src.size() : lineEnd + 1;
        const auto versionLine = 1 + std::count(src.begin(), src.begin() + static_cast<std::ptrdiff_t>(versionPos), '\n');
        // WebGL requires #version on the very first line, so what precedes it (blank lines,
        // comments) goes; "#line n" (the next line is line n) keeps the original numbers.
        result.source = header + "#line " + std::to_string(versionLine + 1) + "\n" + src.substr(lineEnd);
    }
    return result;
}

std::string ivf::prepareShaderSource(std::string_view source, std::vector<UniformDefault> *defaults)
{
    if (defaults != nullptr)
        defaults->clear();
    if (!RenderContext::isGles())
        return std::string(source);

    auto translated = translateToGlslEs(source);
    for (const auto &warning : translated.warnings)
        logWarning(warning, "GLSL ES");
    if (defaults != nullptr)
        *defaults = std::move(translated.defaults);
    return std::move(translated.source);
}

void ivf::applyUniformDefaults(unsigned int program, const std::vector<UniformDefault> &defaults)
{
    if (defaults.empty() || program == 0)
        return;

    GLint previous = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previous);
    glUseProgram(program);
    for (const auto &d : defaults)
    {
        const GLint location = glGetUniformLocation(program, d.name.c_str());
        const int n = componentCount(d.type);
        const int count = std::max(1, d.arraySize);
        if (location < 0 || n == 0 || static_cast<int>(d.values.size()) != n * count)
            continue;

        if (d.type == "uint" || d.type.starts_with("uvec"))
        {
            std::vector<GLuint> v;
            for (double x : d.values)
                v.push_back(static_cast<GLuint>(x));
            if (n == 1) glUniform1uiv(location, count, v.data());
            else if (n == 2) glUniform2uiv(location, count, v.data());
            else if (n == 3) glUniform3uiv(location, count, v.data());
            else glUniform4uiv(location, count, v.data());
        }
        else if (isIntegerType(d.type))
        {
            std::vector<GLint> v;
            for (double x : d.values)
                v.push_back(static_cast<GLint>(x));
            if (n == 1) glUniform1iv(location, count, v.data());
            else if (n == 2) glUniform2iv(location, count, v.data());
            else if (n == 3) glUniform3iv(location, count, v.data());
            else glUniform4iv(location, count, v.data());
        }
        else
        {
            std::vector<GLfloat> v;
            for (double x : d.values)
                v.push_back(static_cast<GLfloat>(x));
            if (n == 1) glUniform1fv(location, count, v.data());
            else if (n == 2) glUniform2fv(location, count, v.data());
            else if (n == 3) glUniform3fv(location, count, v.data());
            else glUniform4fv(location, count, v.data());
        }
    }
    glUseProgram(static_cast<GLuint>(previous));
}
