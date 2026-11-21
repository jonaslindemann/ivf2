#include <ivf/result.h>

#include <ivf/logger.h>

namespace ivf {

const char *toString(ErrorCode code)
{
    switch (code)
    {
    case ErrorCode::FileNotFound: return "file not found";
    case ErrorCode::ReadFailed: return "read failed";
    case ErrorCode::WriteFailed: return "write failed";
    case ErrorCode::UnsupportedFormat: return "unsupported format";
    case ErrorCode::InvalidData: return "invalid data";
    case ErrorCode::CompileFailed: return "compile failed";
    case ErrorCode::LinkFailed: return "link failed";
    case ErrorCode::InvalidArgument: return "invalid argument";
    case ErrorCode::NotSupported: return "not supported";
    default: return "unknown error";
    }
}

std::unexpected<Error> makeError(ErrorCode code, std::string message, std::string_view context)
{
    logError(message, std::string(context));
    return std::unexpected<Error>(Error{code, std::move(message)});
}

} // namespace ivf
