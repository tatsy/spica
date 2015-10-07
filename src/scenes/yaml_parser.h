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
    };

    struct YamlNode {
        int indent;
        std::string val;
        TokenKind kind;
        YamlNode* child;
        YamlNode* sibling;
        
        YamlNode();
        ~YamlNode();
    };

    class SPICA_YAML_PARSER_DLL YamlElement {
    private:
        YamlNode* _ptr;

    private:
        YamlElement(YamlNode* ptr);

    public:
        YamlElement();
        YamlElement(const YamlElement& elem);
        ~YamlElement();
        YamlElement& operator=(const YamlElement& elem);

        bool isNull() const;
        bool hasChild() const;
        YamlElement firstChild() const;
        
        //! get the first child node with specified key
        YamlElement childByKey(const std::string& key) const;

        //! get the value node whose parent has the specified key
        YamlElement valueByKey(const std::string& key) const;

        bool hasSibling() const;
        YamlElement nextSibling() const;

        int asInteger() const;
        bool asBool() const;
        double asDouble() const;
        std::string asString() const;
        std::vector<int> asIntegerList() const;
        std::vector<double> asDoubleList() const;
        std::vector<std::string> asStringList() const;

        friend class YamlParser;
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
        YamlElement documentElement() const;

    private:
        void release();
        YamlNode* recursiveLoad();
        YamlNode* nextToken();

        void print() const;
    };

}  // namespace spica

#endif  // _SPICA_YAML_PARSER_H_
