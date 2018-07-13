#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_EXCEPTION_H
#define SPICA_EXCEPTION_H

#include <cstdio>
#include <cstdarg>
#include <string>
#include <stdexcept>

#include "core/common.h"

namespace spica {

class SPICA_EXPORTS RuntimeException : public std::exception {
public:
    RuntimeException() noexcept;
    RuntimeException(const std::string &message) noexcept;
    RuntimeException(const char *format, ...) noexcept;
    RuntimeException(const RuntimeException &e) noexcept;
    RuntimeException &operator=(const RuntimeException &e) noexcept;

    virtual ~RuntimeException();
    virtual const char *what() const noexcept override;

private:
    std::string message_;
};

}  // namespace spica

#endif //SPICA_EXCEPTION_H
