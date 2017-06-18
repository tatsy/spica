#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_COBJECT_H_
#define _SPICA_COBJECT_H_

#include "renderparams.h"

#define SPICA_EXPORT_PLUGIN(name, descr) \
    extern "C" { \
        void SPICA_EXPORTS *createInstance(const RenderParams &params) { \
            return new name(params); \
        } \
        const char SPICA_EXPORTS *getDescription() { \
            return descr; \
        } \
    }

#endif  // _SPICA_COBJECT_H_
