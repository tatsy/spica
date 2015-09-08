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

    enum class TokenKind : int {
        Key     = 0x01,
        Value   = 0x02,
        Comment = 0x03,
        End     = 0xff
    };

    struct SPICA_YAML_PARSER_DLL YamlNode {
        int indent;
        std::string val;
        TokenKind kind;
        YamlNode* child;
        YamlNode* sibling;
        
        YamlNode();
        ~YamlNode();
    };

    class SPICA_YAML_PARSER_DLL YamlParser : private Uncopyable {
    private:
        int _lpos;
        std::vector<std::string> _lines;
        std::vector<YamlNode*> _nodes;

    public:
        YamlParser();
        YamlParser(const std::string& filename);
        ~YamlParser();

        void load(const std::string& filename);

    private:
        void release();
        YamlNode* recursiveLoad();
        YamlNode* nextToken();

        void print() const;
    };

}  // namespace spica

#endif  // _SPICA_YAML_PARSER_H_
