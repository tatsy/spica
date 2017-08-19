#define SPICA_API_EXPORT
#include "cobject.h"

#if (defined(WIN32) || defined(_WIN32) || defined(WINCE) || defined(__CYGWIN__))
#include <Windows.h>
#define ModuleHandle HINSTANCE
#define LoadModule(MODULE_NAME) LoadLibrary(("plugins/" + MODULE_NAME + ".dll").c_str());
#define GetSymbol(HANDLE, SYMBOL_NAME) GetProcAddress(HANDLE, SYMBOL_NAME)
#else
#include <dlfcn.h>
#define ModuleHandle void*
#define LoadModule(MODULE_NAME) dlopen(("plugins/" + MODULE_NAME + ".so").c_str(), RTLD_LAZY)
#define GetSymbol(HANDLE, SYMBOL_NAME) dlsym(HANDLE, SYMBOL_NAME)
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

    ModuleHandle hModule = LoadModule(moduleName);
    Assertion(hModule != NULL, "Failed to load module: %s", moduleName.c_str());

    ObjectInitializer initializer = (ObjectInitializer)GetSymbol(hModule, "createInstance");
    Assertion(initializer != NULL,
        "The method \"createInstance\" is not defined for module: %s", moduleName.c_str());
    registerInitializer(moduleName, initializer);
}

void PluginManager::initAccelerator(const std::string &moduleName) {
    ModuleHandle hModule = LoadModule(moduleName);
    Assertion(hModule != NULL, "Failed to load module: %s", moduleName.c_str());

    AcceleratorInitializer initializer = (AcceleratorInitializer)GetSymbol(hModule, "createInstance");
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
