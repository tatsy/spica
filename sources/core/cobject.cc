#define SPICA_API_EXPORT
#include "cobject.h"

#if (defined(WIN32) || defined(_WIN32) || defined(WINCE) || defined(__CYGWIN__))
#include <Windows.h>
#elif defined(__GNUC__) && __GNUC__ >= 4
#else
#endif

#include "accelerator.h"

namespace spica {

CObject::CObject() {}

CObject::~CObject() {}

PluginManager::PluginManager() {
}

PluginManager &PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

void PluginManager::initModule(const std::string &moduleName) {
    HINSTANCE hModule = LoadLibrary(("plugins/" + moduleName + ".dll").c_str());
    Assertion(hModule != NULL, "Failed to load module: %s", moduleName.c_str());

    ObjectInitializer initializer = (ObjectInitializer)GetProcAddress(hModule, "createInstance");
    Assertion(initializer != NULL,
        "The method \"createInstance\" is not defined for module: %s", moduleName.c_str());
    registerInitializer(moduleName, initializer);
}

void PluginManager::initAccelerator(const std::string &moduleName) {
    HINSTANCE hModule = LoadLibrary(("plugins/" + moduleName + ".dll").c_str());
    Assertion(hModule != NULL, "Failed to load module: %s", moduleName.c_str());

    AcceleratorInitializer initializer = (AcceleratorInitializer)GetProcAddress(hModule, "createInstance");
    Assertion(initializer != NULL,
        "The method \"createInstance\" is not defined for module: %s", moduleName.c_str());
    registerInitializer(moduleName, initializer);
}

void PluginManager::registerInitializer(const std::string &name, ObjectInitializer initializer) {
    initializers_[name] = initializer;
}

void PluginManager::registerInitializer(const std::string &name, AcceleratorInitializer initializer) {
    accelInitializers_[name] = initializer;
}

CObject *PluginManager::createObject(const std::string &name, RenderParams &params) const {
    auto it = initializers_.find(name);
    Assertion(it != initializers_.cend(),
        "The method \"createInstance\" is not defined for module: %s", name.c_str());

    return (*it->second)(params);
}

Accelerator *PluginManager::createAccelerator(const std::string &name,
                                              const std::vector<std::shared_ptr<Primitive>> &primitives,
                                              RenderParams &params) const {
    auto it = accelInitializers_.find(name);
    Assertion(it != accelInitializers_.cend(),
        "The method \"createInstance\" is not defined for module: %s", name.c_str());

    return (*it->second)(primitives, params);
}

}  // namespace spica