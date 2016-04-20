#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDER_PARAMETERS_H_
#define _SPICA_RENDER_PARAMETERS_H_

#include <string>
#include <map>

namespace spica {

class RenderParams {
public:
    RenderParams()
        : table_{} {
        // Set default parameters
        set("MAX_BOUNCES", 32);
        set("NUM_SAMPLES", 32);
        set("CAST_PHOTONS", 500000);
    }

    void clear() {
        table_.clear();
    }

    template <class T>
    void set(const std::string& key, const T& value);

    template <>
    void set(const std::string& key, const int& value) {
        char buf[32];
        sprintf(buf, "%d", value);
        table_.insert(std::make_pair(key, std::string(buf)));
    }

    template <>
    void set(const std::string& key, const double& value) {
        char buf[32];
        sprintf(buf, "%f", value);
        table_.insert(std::make_pair(key, std::string(buf)));
    }

    template <>
    void set(const std::string& key, const std::string& value) {
        table_.insert(std::make_pair(key, value));
    }

    template <class T>
    T get(const std::string& key) const;

    template <>
    int get(const std::string& key) const {
        const auto it = table_.find(key);
        if (it == table_.cend()) {
            fprintf(stderr, "Specified key \"%s\" was not found\n",
                    key.c_str());
            return 0;
        }
        return std::atoi(it->second.c_str());
    }

    template <>
    double get(const std::string& key) const {
        const auto it = table_.find(key);
        if (it == table_.cend()) {
            fprintf(stderr, "Specified key \"%s\" was not found\n",
                    key.c_str());
            return 0;
        }
        return std::atof(it->second.c_str());
    }

    template <>
    std::string get(const std::string& key) const {
        const auto it = table_.find(key);
        if (it == table_.cend()) {
            fprintf(stderr, "Specified key \"%s\" was not found\n",
                    key.c_str());
            return 0;
        }
        return std::move(it->second);
    }

private:
    // Private fields
    std::map<std::string, std::string> table_;
};

}  // namespace spica

#endif  // _SPICA_RENDER_PARAMETERS_H_
