#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_COBJECT_H_
#define _SPICA_COBJECT_H_

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

#include "common.h"
#include "renderparams.h"

namespace spica {

using ObjectInitializer = CObject *(*)(const RenderParams&);

class SPICA_EXPORTS CObject {
public:
    CObject();
    virtual ~CObject();
};

class SPICA_EXPORTS PluginManager {
public:
    static PluginManager &getInstance();

    void initModule(const std::string &moduleName);
    CObject *createObject(const std::string &name, const RenderParams &params) const;

private:
    PluginManager();

    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;

    void registerInitializer(const std::string &name, ObjectInitializer initializer);

    std::unordered_map<std::string, ObjectInitializer> initializers_; 
};

}  // namespace spica

#define SPICA_EXPORT_PLUGIN(name, descr) \
    extern "C" { \
        CObject SPICA_EXPORTS *createInstance(const RenderParams &params) { \
            return (CObject *)(new name(params)); \
        } \
        const char SPICA_EXPORTS *getDescription() { \
            return descr; \
        } \
    }



#endif  // _SPICA_COBJECT_H_
