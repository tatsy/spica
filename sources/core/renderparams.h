#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDER_PARAMETERS_H_
#define _SPICA_RENDER_PARAMETERS_H_

#include <string>
#include <map>

#include "common.h"

namespace spica {

class SPICA_EXPORTS RenderParams {
public:
    RenderParams();

    void clear();

    template <class T>
    SPICA_EXPORTS void set(const std::string& key, const T& value);

    template <class T>
    SPICA_EXPORTS T get(const std::string& key) const;

private:
    // Private fields
    std::map<std::string, std::string> table_;
};

// -----------------------------------------------------------------------------
// Template specialization for getter/setter methods
// -----------------------------------------------------------------------------

template <>
inline void RenderParams::set(const std::string& key, const int& value) {
    char buf[32];
    sprintf(buf, "%d", value);
    table_[key] = std::string(buf);
}

template <>
inline void RenderParams::set(const std::string& key, const double& value) {
    char buf[32];
    sprintf(buf, "%f", value);
    table_[key] = std::string(buf);
}

template <>
inline void RenderParams::set(const std::string& key, const std::string& value) {
    table_[key] = value;
}

template <>
inline int RenderParams::get(const std::string& key) const {
    const auto it = table_.find(key);
    if (it == table_.cend()) {
        fprintf(stderr, "Specified key \"%s\" was not found\n",
                key.c_str());
        return 0;
    }
    return std::atoi(it->second.c_str());
}

template <>
inline double RenderParams::get(const std::string& key) const {
    const auto it = table_.find(key);
    if (it == table_.cend()) {
        fprintf(stderr, "Specified key \"%s\" was not found\n",
                key.c_str());
        return 0;
    }
    return std::atof(it->second.c_str());
}

template <>
inline std::string RenderParams::get(const std::string& key) const {
    const auto it = table_.find(key);
    if (it == table_.cend()) {
        fprintf(stderr, "Specified key \"%s\" was not found\n",
                key.c_str());
        return 0;
    }
    return std::move(it->second);
}

}  // namespace spica

#endif  // _SPICA_RENDER_PARAMETERS_H_
