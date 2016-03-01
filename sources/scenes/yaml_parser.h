#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_YAML_PARSER_H_
#define _SPICA_YAML_PARSER_H_

#include <iostream>
#include <string>
#include <vector>

#include "../core/common.h"
#include "../core/uncopyable.h"

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

    class SPICA_EXPORTS YamlElement {
    private:
        YamlNode* _ptr;

    private:
        explicit YamlElement(YamlNode* ptr);

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

    class SPICA_EXPORTS YamlParser : private Uncopyable {
    private:
        int _lpos;
        std::vector<std::string> _lines;
        std::vector<YamlNode*> _nodes;

    public:
        YamlParser();
        explicit YamlParser(const std::string& filename);
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
