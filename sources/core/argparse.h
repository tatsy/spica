#ifdef _MSC_VER
#pragma once
#endif

#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

namespace spica {

class ArgumentParser {
private:
    struct Arg {
        Arg(char shortName, const std::string &longName, const std::string &value, bool required)
            : shortName{ shortName }
            , longName{ longName }
            , value{ value }
            , required { required } {
        }

        char shortName;
        std::string longName;
        std::string value;
        bool required;
    };

public:
    ArgumentParser() = default;

    virtual ~ArgumentParser() {
    }

    template <typename T>
    void addArgument(const std::string &shortName, const std::string &longName, T value, bool required = false) {
        std::string v = std::to_string(value);
        addArgument<std::string>(shortName, longName, v, required);
    }

    bool parse(int argc, char **argv) {
        appName = std::string(argv[0]);
        for (int i = 1; i < argc; i++) {
            if (!isValidShort(argv[i]) && !isValidLong(argv[i])) {
                fprintf(stderr, "[WARNING] Unparsed argument: \"%s\"\n", argv[i]);
                continue;
            }
            
            std::string longName = "";
            if (isValidShort(argv[i])) {
                const auto &it = short2long.find(parseShort(argv[i]));
                if (it == short2long.cend()) {
                    fprintf(stderr, "[WARNING] Unknown flag %s!\n", argv[i]);
                    continue;
                }
                longName = it->second;
            } else {
                longName = parseLong(argv[i]);
            }

            if (table.find(longName) != table.cend()) {
                values[longName] = std::string(argv[i + 1]);
                i += 1;
            }
        }

        // Check requirements
        bool success = true;
        for (const auto &arg : args) {
            if (arg.required) {
                const auto it = values.find(arg.longName);
                if (it == values.cend()) {
                    success = false;
                    fprintf(stderr, "Required argument \"%s\" is not specified!\n", arg.longName.c_str());
                }
            }
        }

        return success;
    }

    std::string helpText() {
        std::stringstream ss;
        ss << appName << " ";

        // First, print required arguments.
        for (const auto &arg : args) {
            if (arg.required) {
                ss << ("--" + arg.longName) << " " << toUpper(arg.longName) << " "; 
            }
        }

        // Then, print non-required arguments.
        for (const auto &arg : args) {
            if (!arg.required) {
                ss << "[" << ("--" + arg.longName) << " " << toUpper(arg.longName) << "] ";
            }
        }

        return ss.str();
    }

    int getInt(const std::string &name) const {
        const auto &it = values.find(name);
        if (it == values.cend()) {
            fprintf(stderr, "Unknown name: %s!\n", name.c_str());
            return 0;
        }
        return std::atoi(it->second.c_str());
    }

    double getDouble(const std::string &name) const {
        const auto &it = values.find(name);
        if (it == values.cend()) {
            fprintf(stderr, "Unknown name: %s!\n", name.c_str());
            return 0.0;
        }
        return std::atof(it->second.c_str());
    }

    bool getBool(const std::string &name) const {
        const auto &it = values.find(name);
        if (it == values.cend()) {
            fprintf(stderr, "Unknown name: %s!\n", name.c_str());
            return false;
        }
        
        std::string value = it->second;
        if (value == "True" || value == "true" || value == "Yes" || value == "yes") {
            return true;
        }

        if (value == "False" || value == "false" || value == "No" || value == "no") {
            return false;
        }

        throw std::runtime_error("Cannot parse spacified option to bool!");
    }

    std::string getString(const std::string &name) const {
        const auto &it = values.find(name);
        if (it == values.cend()) {
            fprintf(stderr, "Unknown name: %s!\n", name.c_str());
            return "";
        }
        return it->second;
    }

private:
    // Private methods
    static bool isValidShort(const std::string &name) {
        return name.length() == 2 && name[0] == '-';
    }

    static bool isValidLong(const std::string &name) {
        return name.length() > 2 && name[0] == '-' && name[1] == '-';
    }

    static char parseShort(const std::string &name) {
        if (!isValidShort(name)) {
            throw std::runtime_error("Short name must be a single charactor with a hyphen!");
        }
        return name[1];
    }

    static std::string parseLong(const std::string &name) {
        if (!isValidLong(name)) {
            throw std::runtime_error("Long name must begin with two hyphens!");
        }
        return name.substr(2);
    }

    static std::string toUpper(const std::string &value) {
        std::string ret = value;
        std::transform(ret.begin(), ret.end(), ret.begin(), toupper);
        return ret;
    }

    // Private parameters
    std::string appName;
    std::vector<Arg> args;
    std::unordered_map<std::string, uint32_t> table;
    std::unordered_map<char, std::string> short2long;
    std::unordered_map<std::string, std::string> values;
};

// ---------------------------------------------------------------------------------------------------------------------
// Template specialization
// ---------------------------------------------------------------------------------------------------------------------

template <>
void ArgumentParser::addArgument<std::string>(const std::string &shortName, const std::string &longName,
                                              std::string value, bool required) {
    const char sname = parseShort(shortName);
    const std::string lname = parseLong(longName);
    short2long.insert(std::make_pair(sname, lname));
    table.insert(std::make_pair(lname, args.size()));
    args.emplace_back(sname, lname, value, required);
    if (!required) {
        values.insert(std::make_pair(lname, value));
    }
}

template <>
void ArgumentParser::addArgument<const char*>(const std::string &shortName, const std::string &longName,
                                              const char *value, bool required) {
    addArgument<std::string>(shortName, longName, std::string(value), required);
}

}  // namespace spica

#endif // _ARG_PARSER_H_