#define SPICA_YMAL_PARSER_EXPORT
#include "yaml_parser.h"

#include <iostream>
#include <fstream>
#include <stack>

#include "../utils/common.h"

namespace spica {

    namespace {

        const int kMaxValueLength   = 256;
        const int kMaxKeyLength     = 256;
        const int kMaxCommentLength = 256;

    }  // anonymous namespace

    YamlNode::YamlNode()
        : indent()
        , val()
        , kind()
        , child(nullptr)
        , sibling(nullptr) {
    }

    YamlNode::~YamlNode() {
    }

    YamlParser::YamlParser()
        : _lpos(0)
        , _lines()
        , _nodes() {
    }

    YamlParser::~YamlParser() {
        release();
    }

    void YamlParser::load(const std::string& filename) {
        std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "Failed to load file \"" << filename << "\"" << std::endl;
            std::abort();
        }

        std::string line;
        _lines.clear();
        while (!ifs.eof()) {
            std::getline(ifs, line);
            _lines.push_back(line);
        }
        release();
        YamlNode* root = new YamlNode();
        root->kind = TokenKind::Key;
        root->indent = -1;
        _nodes.push_back(root);

        std::stack<YamlNode*> stk;
        stk.push(root);

        while (_lpos < _lines.size()) {
            YamlNode* node = nextToken();
            if (node == nullptr) {
                // Empty line
                // std::cout << "Empty" << std::endl;
            } else if (node->kind == TokenKind::Comment) {
                // Comment
                // std::cout << "Comment: " << node->val << std::endl;
            } else if (stk.top()->kind == TokenKind::Key &&
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
                } else {
                    std::cerr << "[ERROR] Key-key indentation is invalid!!" << std::endl;
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
        if (_lpos >= _lines.size()) {
            std::cerr << "EOF!!" << std::endl;
            std::abort();
        }

        std::string line = _lines[_lpos++];
        int pos = 0;
        while (pos < line.size() && isspace(line[pos])) pos++;
        if (pos == line.size()) return nullptr;

        YamlNode* node = new YamlNode();
        node->indent = pos;

        char cc = line[pos++];
        if (cc == '#') node->kind = TokenKind::Comment;
        else if (cc == '-') node->kind = TokenKind::Value;
        else {
            pos--;
            node->kind = TokenKind::Key;
        }

        while (pos < line.size() && isspace(line[pos])) pos++;

        int cnt = 0;
        char val[kMaxValueLength + 1];
        while (pos < line.size()) {
            val[cnt++] = line[pos++];
            if (pos >= line.size()) break;
            if (cnt >= kMaxValueLength) break;
        }
        val[cnt] = '\0';

        node->val = std::string(val);
        _nodes.push_back(node);
        return node;
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
