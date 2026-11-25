#pragma once

/**
 * @file readers.h
 * @brief Reading data files: CSV tables, NumPy arrays (.npy) and VTK files (.vtk, .vtu, .vti).
 *
 * All readers return ivf::Result: on failure an ivf::Error says why (file not found, format
 * not supported, malformed data), and the error is logged.
 *
 * @code
 * auto table = ivfvis::readCsv("measurements.csv");
 * if (!table)
 *     return -1;
 * auto points = ivfvis::pointsFromTable(*table, "x", "y", "z");   // other columns become arrays
 *
 * auto grid = ivfvis::gridFromNpy(*ivfvis::readNpy("density.npy"), "density");
 *
 * auto data = ivfvis::readVtk("result.vtu");      // or .vtk (legacy) and .vti
 * auto mesh = ivfvis::createMeshNode(*data->points, data->triangles);
 * @endcode
 */

#include <ivfvis/point_set.h>
#include <ivfvis/uniform_grid.h>

#include <ivf/mesh_node.h>
#include <ivf/result.h>

#include <glm/glm.hpp>

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ivfvis {

// ---------------------------------------------------------------------------------------
// CSV
// ---------------------------------------------------------------------------------------

/** @brief Columns of numbers with names. */
struct Table {
    std::vector<std::string> names;          ///< Column names ("column1", ... without a header).
    std::vector<std::vector<float>> columns; ///< Values by column; NaN where a cell is not a number.

    [[nodiscard]] std::size_t rowCount() const;

    /** @brief Index of column @p name, or -1. */
    [[nodiscard]] int column(std::string_view name) const;

    /** @brief Values of column @p name (empty if there is none). */
    [[nodiscard]] std::span<const float> values(std::string_view name) const;
};

/** @brief Settings of readCsv(). */
struct CsvOptions {
    char delimiter{0};  ///< 0 (default): detected from the first data line (',', ';', tab or spaces).
    int header{-1};     ///< -1 (default): detected (first row not all numbers); 0: none; 1: first row.
    char comment{'#'};  ///< Lines starting with this are skipped (0: none).
};

/** @brief Read a delimited text file of numbers (quoted fields are supported). */
[[nodiscard]] ivf::Result<Table> readCsv(const std::filesystem::path &path, const CsvOptions &options = {});

/**
 * @brief Points with coordinates from columns @p x, @p y and @p z (an empty name gives 0);
 * every other column with at least one number becomes a scalar array.
 */
[[nodiscard]] ivf::Result<PointSetPtr> pointsFromTable(const Table &table, std::string_view x, std::string_view y,
                                                       std::string_view z);

// ---------------------------------------------------------------------------------------
// NumPy
// ---------------------------------------------------------------------------------------

/** @brief An array read from a .npy file, converted to floats. */
struct NpyArray {
    std::vector<std::size_t> shape; ///< Shape as in NumPy.
    std::vector<float> data;        ///< Values in file order.
    bool fortranOrder{false};       ///< True if the first index varies fastest.
    std::string dtype;              ///< NumPy type string, e.g. "<f4".

    [[nodiscard]] std::size_t size() const;
};

/** @brief Read a .npy file (version 1-3; bool, integer and float types, either byte order). */
[[nodiscard]] ivf::Result<NpyArray> readNpy(const std::filesystem::path &path);

/**
 * @brief A UniformGrid holding @p array as scalar array @p name.
 *
 * Shapes (C order): (nx), (ny, nx), (nz, ny, nx) as scalars, (nz, ny, nx, 3) as vectors - the
 * layout of NumPy arrays indexed [k][j][i]. Fortran-ordered arrays use (nx, ny, nz[, 3]).
 */
[[nodiscard]] ivf::Result<UniformGridPtr> gridFromNpy(const NpyArray &array, std::string_view name,
                                                      const glm::vec3 &origin = glm::vec3(0.0f),
                                                      const glm::vec3 &spacing = glm::vec3(1.0f));

/**
 * @brief Points from an array of shape (n, 2) or (n, k >= 3): the first columns are x, y (and
 * z); further columns become scalar arrays "column3", "column4", ...
 */
[[nodiscard]] ivf::Result<PointSetPtr> pointsFromNpy(const NpyArray &array);

// ---------------------------------------------------------------------------------------
// VTK
// ---------------------------------------------------------------------------------------

/**
 * @brief Contents of a VTK file.
 *
 * Image data (structured points, .vti) gives a @c grid with the point arrays. Polygonal and
 * unstructured data give @c points with the point arrays, @c triangles for their surface
 * (polygons, triangle strips, quads and pixels split into triangles; the outer faces of
 * volume cells) and @c lines for line and polyline cells. Cell data is not read.
 */
struct VtkData {
    UniformGridPtr grid;
    PointSetPtr points;
    std::vector<glm::uvec3> triangles;
    std::vector<glm::uvec2> lines;
};

/**
 * @brief Read a VTK file: legacy .vtk (versions 2-5.1, ASCII and binary; structured points,
 * polydata, unstructured, structured and rectilinear grids), or XML .vtu (unstructured grid)
 * and .vti (image data) with ASCII, base64 or appended data, raw or zlib-compressed.
 *
 * XML files need ivf2 built with IVF2_WITH_VTK_XML (default); otherwise they give an
 * ErrorCode::NotSupported error.
 */
[[nodiscard]] ivf::Result<VtkData> readVtk(const std::filesystem::path &path);

/**
 * @brief A mesh node of the triangles over @p points, with smooth normals; vertex i is point
 * i, so colorVertices() with the point set's arrays colours it.
 */
[[nodiscard]] ivf::MeshNodePtr createMeshNode(const PointSet &points, std::span<const glm::uvec3> triangles);

} // namespace ivfvis
