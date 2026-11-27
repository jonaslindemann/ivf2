#pragma once
// Minimal, dependency-free test harness for ivf2's CTest suite.
// Each test executable registers cases with IVF_TEST and returns non-zero on failure.

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace ivftest {

struct Case {
    const char *name;
    std::function<void()> fn;
};

inline std::vector<Case> &registry()
{
    static std::vector<Case> cases;
    return cases;
}

inline int &failures()
{
    static int n = 0;
    return n;
}

struct Registrar {
    Registrar(const char *name, std::function<void()> fn) { registry().push_back({name, std::move(fn)}); }
};

inline int runAll()
{
    for (auto &c : registry())
    {
        int before = failures();
        c.fn();
        std::printf("[%s] %s\n", failures() == before ? " OK " : "FAIL", c.name);
        std::fflush(stdout); // a crash in a later test keeps this line
    }
    std::printf("%zu test(s), %d failed check(s)\n", registry().size(), failures());
    return failures() == 0 ? 0 : 1;
}

} // namespace ivftest

#define IVF_TEST_CAT2(a, b) a##b
#define IVF_TEST_CAT(a, b) IVF_TEST_CAT2(a, b)

#define IVF_TEST(name)                                                                                                 \
    static void name();                                                                                                \
    static ivftest::Registrar IVF_TEST_CAT(reg_, name)(#name, name);                                                   \
    static void name()

#define IVF_CHECK(cond)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            std::printf("  %s:%d: check failed: %s\n", __FILE__, __LINE__, #cond);                                    \
            ++ivftest::failures();                                                                                     \
        }                                                                                                              \
    } while (0)

#define IVF_CHECK_NEAR(a, b, eps) IVF_CHECK(std::fabs(double(a) - double(b)) <= double(eps))

#define IVF_TEST_MAIN()                                                                                                \
    int main()                                                                                                         \
    {                                                                                                                  \
        return ivftest::runAll();                                                                                      \
    }
