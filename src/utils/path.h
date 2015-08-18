#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PATH_H_
#define _SPICA_PATH_H_

#if _MSC_VER
#include <direct.h>
#define mkdir(d) _mkdir(d)
#else
#include <sys/stat.h>
#endif

#include <string>

namespace spica {

    namespace path {

        inline std::string getExtension(const std::string& filename) {
            const int pos = filename.find_last_of(".");
            return filename.substr(pos);
        }

        inline bool createDirectory(const std::string& dirname) {
            return mkdir(dirname.c_str()) == 0;   
        }

    }

}  // namespace spica

#endif  // _SPICA_PATH_H_
