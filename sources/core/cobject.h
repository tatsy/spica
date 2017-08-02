#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_COBJECT_H_
#define _SPICA_COBJECT_H_

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include "common.h"
#include "core.hpp"
#include "renderparams.h"

namespace spica {

using ObjectInitializer = CObject *(*)(RenderParams&);
using AcceleratorInitializer = Accelerator *(*)(const std::vector<std::shared_ptr<Primitive>> &,
                                                RenderParams &);

class SPICA_EXPORTS CObject {
public:
    CObject();
    virtual ~CObject();
};

class SPICA_EXPORTS PluginManager {
public:
    static PluginManager &getInstance();

    void initModule(const std::string &moduleName);
    void initAccelerator(const std::string &moduleName);
    CObject *createObject(const std::string &name, RenderParams &params) const;
    Accelerator *createAccelerator(const std::string &name, 
                                   const std::vector<std::shared_ptr<Primitive>> &primitives,
                                   RenderParams &params) const;

private:
    PluginManager();

    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;

    void registerInitializer(const std::string &name, ObjectInitializer initializer);
    void registerInitializer(const std::string &name, AcceleratorInitializer initializer);

    std::unordered_map<std::string, ObjectInitializer> initializers_;
    std::unordered_map<std::string, AcceleratorInitializer> accelInitializers_;
};

}  // namespace spica

#define SPICA_EXPORT_PLUGIN(name, descr) \
    extern "C" { \
        CObject SPICA_EXPORTS *createInstance(RenderParams &params) { \
            return (CObject *)(new name(params)); \
        } \
        const char SPICA_EXPORTS *getDescription() { \
            return descr; \
        } \
    }

#define SPICA_EXPORT_ACCEL_PLUGIN(name, descr) \
    extern "C" { \
        CObject SPICA_EXPORTS *createInstance(const std::vector<std::shared_ptr<Primitive>> &primitives, \
                                              RenderParams &params) { \
            return (CObject *)(new name(primitives, params)); \
        } \
        const char SPICA_EXPORTS *getDescription() { \
            return descr; \
        } \
    }



#endif  // _SPICA_COBJECT_H_
