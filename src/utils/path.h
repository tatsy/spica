#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PATH_H_
#define _SPICA_PATH_H_

#if _MSC_VER
#include <direct.h>
#define makedir(d) _mkdir(d)
#else
#include <sys/types.h>
#include <sys/stat.h>
#define makedir(d) mkdir(d, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

#include <string>
#include <vector>

namespace spica {

    namespace path {

        inline std::string getExtension(const std::string& filename) {
            const int pos = filename.find_last_of(".");
            return filename.substr(pos);
        }

        inline bool createDirectory(const std::string& dirname) {
            return makedir(dirname.c_str()) == 0;   
        }

        inline std::vector<std::string> split(const std::string& str, const std::string& delim) {
            int prev = 0;
            int pos  = 0;
            std::vector<std::string> retval;
            while ((pos = str.find_first_of(delim, pos)) != std::string::npos) {
                retval.push_back(str.substr(prev, pos));
                prev = pos + delim.size();
            }
            retval.push_back(str.substr(prev));
            return retval;
        }
    }

}  // namespace spica

#endif  // _SPICA_PATH_H_
