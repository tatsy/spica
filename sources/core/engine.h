#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ENGINE_H_
#define _SPICA_ENGINE_H_

#include <string>
#include <memory>
#include <vector>
#include <functional>

#include "common.h"
#include "forward_decl.h"
#include "spectrum.h"

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
    virtual ~Engine();

    void init(const Option& option);
    void start(const std::string& filename) const;
    void cleanup();

protected:
    template <class T>
    bool parse(const boost::property_tree::ptree& xml, T* value) const;

    bool parse_shape(const boost::property_tree::ptree& xml,
                     std::vector<std::shared_ptr<Shape>>* shapes,
                     const Transform& toWorld,
                     const std::string& directory = "") const;

    bool parse_envmap(const boost::property_tree::ptree& xml,
                      std::shared_ptr<Light>* light,
                      const Sphere& worldSphere,
                      const std::string& directory = "") const;

    bool parse_subsurface(const boost::property_tree::ptree& xml,
                          std::shared_ptr<Texture<Spectrum>>* sigma_a,
                          std::shared_ptr<Texture<Spectrum>>* sigma_s,
                          double* eta, double* scale, double* g) const;

    bool parse_subsurface_mtrl(const boost::property_tree::ptree& xml,
                               std::shared_ptr<Material>* mtrl,
                               const std::shared_ptr<Texture<Spectrum>>& sigma_a,
                               const std::shared_ptr<Texture<Spectrum>>& sigma_s,
                               double eta, double scale, double g) const;

    virtual bool parse_film(const boost::property_tree::ptree& xml,
                            Film** film) const;

    bool find_field(const boost::property_tree::ptree& xml,
                    const std::string& field, const std::string& value,
                    boost::property_tree::ptree* result,
                    std::string* tag) const;

    // Private fields
    Option option_;

};  // class Engine

template <>
SPICA_EXPORTS bool Engine::parse(const boost::property_tree::ptree& xml,
    Transform* transform) const;

template <>
SPICA_EXPORTS bool Engine::parse(const boost::property_tree::ptree& xml,
    std::shared_ptr<Camera>* camera) const;

template <>
SPICA_EXPORTS bool Engine::parse(const boost::property_tree::ptree& xml,
    std::shared_ptr<Sampler>* sampler) const;

template <>
SPICA_EXPORTS bool Engine::parse(const boost::property_tree::ptree& xml,
   std::shared_ptr<Material>* material) const;

template <>
SPICA_EXPORTS bool Engine::parse(const boost::property_tree::ptree& xml,
    std::shared_ptr<MediumInterface>* medium) const;

}  // namespace spica

#endif  // _SPICA_ENGINE_H_
