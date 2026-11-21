#pragma once
/**
 * @file result.h
 * @brief Error handling for operations that can fail: ivf::Result<T>.
 *
 * Functions that load or create resources (textures, models, fonts, scenes, ...)
 * return `ivf::Result<T>`, which holds either the value or an `ivf::Error`
 * describing what went wrong. It is `std::expected<T, ivf::Error>` (C++23):
 *
 * @code
 * auto texture = ivf::Texture::fromFile("brick.png");
 * if (!texture)
 * {
 *     showMessage(texture.error().message);   // e.g. "Could not load 'brick.png': file not found"
 *     return;
 * }
 * node->setTexture(*texture);                  // * gives the value
 *
 * node->setTexture(ivf::Texture::fromFile("brick.png").value_or(fallback));
 * @endcode
 *
 * Policy (see also docs: Advanced > Error Handling):
 * - Expected failures (missing files, bad formats, shader compile errors) are
 *   returned as a Result. The error is also logged, so a failure is visible even
 *   when the caller does not check.
 * - Exceptions are only used for programming errors (e.g. a required pointer is null).
 * - `*result` on a failed result is undefined behaviour; `result.value()` throws
 *   std::bad_expected_access. Check first, or use value_or().
 */

#include <expected>
#include <string>
#include <string_view>

namespace ivf {

/** @brief Category of an error, for code that reacts to specific failures. */
enum class ErrorCode {
    FileNotFound,      ///< The file does not exist.
    ReadFailed,        ///< The file exists but could not be read.
    WriteFailed,       ///< A file could not be written.
    UnsupportedFormat, ///< The data is not in a format that can be decoded.
    InvalidData,       ///< The data was read but is malformed or inconsistent.
    CompileFailed,     ///< A shader failed to compile.
    LinkFailed,        ///< A shader program failed to link.
    InvalidArgument,   ///< An argument was out of range or missing.
    NotSupported,      ///< The feature is not available in this build or on this system.
    Unknown            ///< Any other failure.
};

/** @brief Readable name of an error code, e.g. "file not found". */
const char *toString(ErrorCode code);

/** @brief An error: a category and a readable message. */
struct Error {
    ErrorCode code{ErrorCode::Unknown};
    std::string message;
};

/** @brief Either a value of type T or an Error. */
template <typename T> using Result = std::expected<T, Error>;

/**
 * @brief Log an error (at error level, with the given context) and return it as a failed result.
 *
 * Intended for `return makeError(...)` in functions returning `Result<T>`:
 * @code
 * if (!std::filesystem::exists(path))
 *     return makeError(ErrorCode::FileNotFound, "Could not load '" + path + "': file not found", "Texture");
 * @endcode
 */
std::unexpected<Error> makeError(ErrorCode code, std::string message, std::string_view context = "");

/** @brief Like makeError(), but without logging - for errors the caller is expected to report. */
inline std::unexpected<Error> silentError(ErrorCode code, std::string message)
{
    return std::unexpected<Error>(Error{code, std::move(message)});
}

} // namespace ivf
