#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ENGINE_H_
#define _SPICA_ENGINE_H_

#include <string>
#include <memory>

#include "common.h"
#include "../integrator/integrator.h"

namespace spica {

struct Option {
    int nThreads = 1;
    int nSamples = 16;
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
    // Private fields
    Option option_;
    std::unique_ptr<Integrator> integrator_;

};  // class Engine

}  // namespace spica

#endif  // _SPICA_ENGINE_H_
