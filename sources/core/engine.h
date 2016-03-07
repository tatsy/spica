#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ENGINE_H_
#define _SPICA_ENGINE_H_

#include <string>
#include <memory>

#include "common.h"
#include "forward_decl.h"

namespace boost {
namespace property_tree {
template < class Key, class Data, class KeyCompare >
class basic_ptree;
using ptree = basic_ptree<std::string, std::string, std::less<std::string> >;
}
}

namespace spica {

struct Option {
    int nThreads = 1;
    int nSamples = 16;
    bool verbose = true;
    std::string outfile = kOutputDirectory + "image_%03d.png";
};

/**
 * Main rendering engine.
 */
class SPICA_EXPORTS Engine {
public:
    // Public methods
    Engine();
    ~Engine();

    void init(const Option& option);
    void start(const std::string& filename) const;
    void cleanup();

private:
    // Private methods
    void printOut(const char* const format, ...) const;
    void printErr(const char* const format, ...) const;

    bool parseTransform(const boost::property_tree::ptree& xml,
                        Transform* transform) const;

    bool parseCamera(const boost::property_tree::ptree& xml,
                     std::shared_ptr<Camera>* camera,
                     std::shared_ptr<Sampler>* sampler) const;

    bool parseSampler(const boost::property_tree::ptree& xml,
                      std::shared_ptr<Sampler>* sampler) const;

    bool parseFilm(const boost::property_tree::ptree& xml,
                   Film** film) const;

    // Private fields
    Option option_;

};  // class Engine

}  // namespace spica

#endif  // _SPICA_ENGINE_H_
