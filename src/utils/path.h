#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PATH_H_
#define _SPICA_PATH_H_

#include <string>

namespace spica {

    namespace path {

        inline std::string getExtension(const std::string& filename) {
            const int pos = filename.find_last_of(".");
            return filename.substr(pos);
        }

    }

}  // namespace spica

#endif  // _SPICA_PATH_H_
