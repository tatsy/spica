#define SPICA_YMAL_PARSER_EXPORT
#include "yaml_parser.h"

#include <iostream>
#include <fstream>
#include <stack>

#include "../utils/common.h"
#include "../utils/path.h"

namespace spica {

    namespace {

        const int kMaxValueLength   = 256;
        const int kMaxKeyLength     = 256;
        const int kMaxCommentLength = 256;

        std::string trim(const std::string& s) {
            int q = s.size() - 1;
            while (q >= 0 && isspace(s[q])) q--;

            if (q == 0) return "";
            return s.substr(0, q + 1);
        }
        
        std::string removeComment(const std::string& s) {
            int p = 0;
            while (p < s.size() && s[p] != '#') p++;
            if (p != s.size()) return s.substr(0, p);
            return s;
        }

    }  // anonymous namespace


    // -------------------------------------------------------------------------
    // YAML Node (used only build time)
    // -------------------------------------------------------------------------

    YamlNode::YamlNode()
        : indent()
        , val()
        , kind()
        , child(nullptr)
        , sibling(nullptr) {
    }

    YamlNode::~YamlNode() {
    }


    // -------------------------------------------------------------------------
    // YAML Element
    // -------------------------------------------------------------------------

    YamlElement::YamlElement(YamlNode* ptr)
        : _ptr(ptr) {
    }

    bool YamlElement::isNull() const {
        return _ptr == nullptr;
    }

    bool YamlElement::hasChild() const {
        return _ptr->child != nullptr;
    }

    YamlElement YamlElement::firstChild() const {
        return YamlElement(_ptr->child);    
    }

    YamlElement YamlElement::childByKey(const std::string& key) const {
        Assertion(_ptr != nullptr, "Element is null!!");
        YamlNode* child = _ptr->child;
        while (child != nullptr) {
            if (child->val == key) {
                return YamlElement(child);
            }
            std::cout << child->val << std::endl;
            child = child->sibling;
        }
        return YamlElement();
    }

    bool YamlElement::hasSibling() const {
        return _ptr->sibling != nullptr;
    }

    YamlElement YamlElement::nextSibling() const {
        return YamlElement(_ptr->sibling);
    }

    bool YamlElement::asBool() const {
        Assertion(_ptr->child != nullptr && _ptr->child->kind == TokenKind::Value,
                  "Child of the node does not have value");
        std::string& s = _ptr->child->val;
        if (s == "Yes" || s == "True" || s == "yes" || s == "true") {
            return true;
        }
        return false;        
    }

    int YamlElement::asInteger() const {
        Assertion(_ptr->child != nullptr && _ptr->child->kind == TokenKind::Value,
                  "Child of the node does not have value");
        std::string& s = _ptr->child->val;
        return atoi(s.c_str());
    }

    double YamlElement::asDouble() const {
        Assertion(_ptr->child != nullptr && _ptr->child->kind == TokenKind::Value,
                  "Child of the node does not have value");
        std::string& s = _ptr->child->val;
        return atof(s.c_str());
    }

    std::string YamlElement::asString() const {
        Assertion(_ptr->child != nullptr && _ptr->child->kind == TokenKind::Value,
                  "Child of the node does not have value");
        return _ptr->child->val;
    }

    std::vector<int> YamlElement::asIntegerList() const {
        Assertion(_ptr->child != nullptr && _ptr->child->kind == TokenKind::Value,
                  "Child of the node does not have value");
        std::string& s = _ptr->child->val;
        Assertion(s[0] == '[' && s[s.size() - 1] == ']', "List parse error");

        std::vector<std::string> vals = path::split(s.substr(1, s.size() - 2), ",");
        std::vector<int> retval(vals.size());
        for (int i = 0; i < vals.size(); i++) {
            retval[i] = atoi(vals[i].c_str());
        }
        return std::move(retval);
    }

    std::vector<double> YamlElement::asDoubleList() const {
        Assertion(_ptr->child != nullptr && _ptr->child->kind == TokenKind::Value,
                  "Child of the node does not have value");
        std::string& s = _ptr->child->val;
        Assertion(s[0] == '[' && s[s.size() - 1] == ']', "List parse error");

        std::vector<std::string> vals = path::split(s.substr(1, s.size() - 2), ",");
        std::vector<double> retval(vals.size());
        for (int i = 0; i < vals.size(); i++) {
            retval[i] = atof(vals[i].c_str());
        }
        return std::move(retval);        
    }

    std::vector<std::string> YamlElement::asStringList() const {
        Assertion(_ptr->child != nullptr && _ptr->child->kind == TokenKind::Value,
                  "Child of the node does not have value");
        std::string& s = _ptr->child->val;
        Assertion(s[0] == '[' && s[s.size() - 1] == ']', "List parse error");

        std::vector<std::string> vals = path::split(s.substr(1, s.size() - 2), ",");
        return std::move(vals);
    }

    // -------------------------------------------------------------------------
    // YAML Parser
    // -------------------------------------------------------------------------

    YamlParser::YamlParser()
        : _lpos(0)
        , _lines()
        , _nodes() {
    }

    YamlParser::YamlParser(const std::string& filename)
        : _lpos(0)
        , _lines()
        , _nodes() {
        load(filename);
    }

    YamlParser::~YamlParser() {
        release();
    }

    void YamlParser::load(const std::string& filename) {
        // Open file
        std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "Failed to load file \"" << filename << "\"" << std::endl;
            std::abort();
        }

        // Read lines
        std::string line;
        _lines.clear();
        while (!ifs.eof()) {
            std::getline(ifs, line);
            line = removeComment(line);
            line = trim(line);

            if (line != "") {
                _lines.push_back(line);
            }
        }

        // Clear previous data
        release();

        // Tree construction
        YamlNode* root = new YamlNode();
        root->kind = TokenKind::Key;
        root->indent = -1;
        _nodes.push_back(root);

        std::stack<YamlNode*> stk;
        stk.push(root);
        while (_lpos < _lines.size()) {
            YamlNode* node = nextToken();
            if (stk.top()->kind == TokenKind::Key &&
                node->kind == TokenKind::Key) {
                // Key-Key
                if (stk.top()->indent < node->indent) {
                    // Child
                    stk.top()->child = node;
                    stk.push(node);
                } else if (stk.top()->indent == node->indent) {
                    // Sibling
                    stk.top()->sibling = node;
                    stk.push(node);
                } else if (stk.top()->child != nullptr) {
                    while (!stk.empty() &&
                           stk.top()->child != nullptr &&
                           stk.top()->indent > node->indent) {
                        stk.pop();
                    }
                    stk.top()->sibling = node;
                    stk.push(node);
                } else {
                    std::cerr << "[ERROR] Key-key indentation is invalid!!"
                              << std::endl;
                    std::abort();
                }
            } else if (stk.top()->kind == TokenKind::Key &&
                       node->kind == TokenKind::Value) {
                // Key-Value
                Assertion(stk.top()->child == nullptr &&
                          stk.top()->indent < node->indent,
                          "Key-value relationship is invalid");
                stk.top()->child = node;
                stk.push(node);
            } else if (stk.top()->kind == TokenKind::Value &&
                       node->kind == TokenKind::Key) {
                // Value-Key
                Assertion(stk.top()->indent > node->indent,
                          "Value-key indentation is invalid");
                while (stk.top()->indent != node->indent) stk.pop();
                stk.top()->sibling = node;
                stk.push(node);
            } else if (stk.top()->kind == TokenKind::Value &&
                       node->kind == TokenKind::Value) {
                // Value-Value
                Assertion(stk.top()->indent == node->indent,
                          "Value indentation is invalid");
                stk.top()->sibling = node;
                stk.push(node);
            }
        }
        print();
    }

    void YamlParser::release() {
        for (int i = 0; i < _nodes.size(); i++) {
            delete _nodes[i];
        }
        _nodes.clear();
    }

    YamlNode* YamlParser::nextToken() {
        Assertion(_lpos < _lines.size(), "You may already finish reading");

        std::string line = _lines[_lpos++];
        int pos = 0;
        while (pos < line.size() && isspace(line[pos])) pos++;
        if (pos == line.size()) return nullptr;

        YamlNode* node = new YamlNode();
        node->indent = pos;

        char cc = line[pos++];
        if (cc == '-') node->kind = TokenKind::Value;
        else {
            pos--;
            node->kind = TokenKind::Key;
        }

        // Skip intermediate white spaces
        while (pos < line.size() && isspace(line[pos])) pos++;

        int cnt = 0;
        char buf[kMaxValueLength + 1];
        while (pos < line.size()) {
            buf[cnt++] = line[pos++];
            if (pos >= line.size()) break;
            if (cnt >= kMaxValueLength) break;
        }
        buf[cnt] = '\0';

        std::string val(buf);
        if (node->kind == TokenKind::Key) {
            int cp = val.find(':');
            Assertion(cp != std::string::npos, "Key expression is invalid");

            // Process expression like "key: value"
            node->val = val.substr(0, cp);

            cp++;
            while (cp < val.size() && isspace(cp)) cp++;

            if (cp < val.size()) {
                YamlNode* child = new YamlNode();
                child->indent = node->indent + 2;
                child->val = val.substr(cp);
                child->kind = TokenKind::Value;
                node->child = child;
                _nodes.push_back(child);
            }

        } else {
            node->val = val;
        }

        _nodes.push_back(node);
        return node;
    }

    YamlElement YamlParser::documentElement() const {
        return YamlElement(_nodes[0]);
    }

    void YamlParser::print() const {
        std::stack<YamlNode*> stk;
        stk.push(_nodes[0]);
        while (!stk.empty()) {
            YamlNode* node = stk.top();
            stk.pop();
            
            if (node->sibling != nullptr) {
                stk.push(node->sibling);
            }

            if (node->child != nullptr) {
                stk.push(node->child);
            }
            
            
            for (int i = 0; i < node->indent; i++) printf(" ");
            printf("%s\n", node->val.c_str());
        }
    }

}  // namespace spica
