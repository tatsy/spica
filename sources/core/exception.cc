#define SPICA_API_EXPORT
#include "exception.h"

namespace spica {

RuntimeException::RuntimeException() noexcept
    : std::exception{}
    , message_{} {
}

RuntimeException::RuntimeException(const std::string &message) noexcept
    : std::exception{}
    , message_{message} {
}

RuntimeException::RuntimeException(const char *format, ...) noexcept
    : RuntimeException{} {
    std::va_list argList;
    va_start(argList, format);

    char msgBytes[512];
    vsprintf(msgBytes, format, argList);
    va_end(argList);

    message_ = std::string(msgBytes);
}

RuntimeException::RuntimeException(const RuntimeException &e) noexcept
    : RuntimeException{e.message_} {
}

RuntimeException::~RuntimeException() {
}

RuntimeException &RuntimeException::operator=(const RuntimeException & e) noexcept {
    this->message_ = e.message_;
    return *this;
}

const char *RuntimeException::what() const noexcept {
    return message_.c_str();
}

}  // namespace spica