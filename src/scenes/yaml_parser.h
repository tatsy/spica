#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_YAML_PARSER_H_
#define _SPICA_YAML_PARSER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_YAML_PARSER_EXPORT
        #define SPICA_YAML_PARSER_DLL __declspec(dllexport)
    #else
        #define SPICA_YAML_PARSER_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_YAML_PARSER_DLL
#endif

#include <iostream>
#include <string>
#include <vector>

#include "../utils/uncopyable.h"

namespace spica {

    class SPICA_YAML_PARSER_DLL YamlNode {
    private:
        std::string val;
        std::vector<int> children;

        friend class YamlParser;
    };

    class SPICA_YAML_PARSER_DLL YamlParser : private Uncopyable {
    private:
        enum class TokenKind : int {
            Key     = 0x01,
            Value   = 0x02,
            Comment = 0x03,
            End     = 0xff
        };

        struct Token {
            std::string value;
            TokenKind kind;
            Token(const std::string& v = "", TokenKind k = TokenKind::End)
                : value(v)
                , kind(k) {
            }
        };

    public:
        YamlParser();
        YamlParser(const std::string& filename);
        ~YamlParser();

        void load(const std::string& filename);

    private:
        int countIndent(std::ifstream& ifs) const;
        void skipSpace(std::ifstream& ifs) const;
        void skipLine(std::ifstream& ifs) const;
        Token getNextToken(std::ifstream& ifs) const;
        std::string getNextKey(std::ifstream& ifs) const;
        std::string getNextComment(std::ifstream& ifs) const;
        std::string getNextValue(std::ifstream& ifs) const;
        std::string getNextValueIfExist(std::ifstream& ifs) const;
    };

}  // namespace spica

#endif  // _SPICA_YAML_PARSER_H_
