#define SPICA_YMAL_PARSER_EXPORT
#include "yaml_parser.h"

#include <iostream>
#include <fstream>

namespace spica {

    namespace {
        const int kMaxValueLength   = 256;
        const int kMaxKeyLength     = 256;
        const int kMaxCommentLength = 256;
    }

    YamlParser::YamlParser() {
    }

    YamlParser::~YamlParser() {
    }

    void YamlParser::load(const std::string& filename) {
        printf("%s\n", isspace('\n') ? "YES" : "NO");
        printf("%s\n", isspace('\r') ? "YES" : "NO");

        std::ifstream ifs(filename.c_str(), std::ios::in);

        std::string line;
        while(!ifs.eof()) {
            int indent = countIndent(ifs);
            if (ifs.eof()) break;

            Token tkn  = getNextToken(ifs);
            fprintf(stdout, "%d: %d, <<%s>>\n", static_cast<int>(tkn.kind), indent, tkn.value.c_str());
            if (tkn.kind == TokenKind::Key) {
                std::string tkn2 = getNextValueIfExist(ifs);
                std::cout << tkn2 << std::endl;
            }

            if (tkn.kind == TokenKind::End) break;
        }
    }

    int YamlParser::countIndent(std::ifstream& ifs) const {
        char c;
        int indent = 0;
        while(!ifs.eof()) {
            c = ifs.get();
            if (isspace(c)) {
                indent++;
                if (c == '\r' || c == '\n') return -1;
            } else {
                ifs.unget();
                break;
            }
        }
        return indent;
    }

    void YamlParser::skipSpace(std::ifstream& ifs) const {
        char c;
        while (!ifs.eof()) {
            c = ifs.get();
            if (!isspace(c)) {
                ifs.unget();
                return;
            }
        }
    }

    void YamlParser::skipLine(std::ifstream& ifs) const {
        char c;
        while (!ifs.eof()) {
            c = ifs.get();
            if (c == '\r') {
                if (!ifs.eof()) {
                    char cc = ifs.get();
                    if (cc == '\n') {
                        // Windows (CRLF)
                        return;
                    } else {
                        // Mac (CR)
                        ifs.unget();
                        return;
                    }
                }
            } else if (c == '\n') {
                // Linux (LF)
                return;
            }
        }
    }

    YamlParser::Token YamlParser::getNextToken(std::ifstream& ifs) const {
        skipSpace(ifs);

        char c;
        while (!ifs.eof()) {
            c = ifs.get();
            if (!isspace(c)) break;
        }

        Token tkn;
        if (c != EOF) {
            if (c == '#') { 
                // Comment
                std::string comment = getNextComment(ifs);
                tkn = Token(comment, TokenKind::Comment);
            } else if (c == '-') {
                // Value
                std::string val = getNextValue(ifs);
                tkn = Token(val, TokenKind::Value);
                skipLine(ifs);
            } else if (isalpha(c) || isdigit(c) || c == '-') {
                // Key
                ifs.unget();
                std::string key = getNextKey(ifs);
                tkn = Token(key, TokenKind::Key);
            } else {
                fprintf(stderr, "[ERROR] invalid charactor \"%c\" is detected !!\n", c);
                fprintf(stderr, "%d\n", (int)c);
                std::abort();
            }
        }

        return tkn;
    }

    std::string YamlParser::getNextKey(std::ifstream& ifs) const {
        skipSpace(ifs);

        // Key
        int cnt = 0;
        char c;
        char key[kMaxKeyLength];
        while(!ifs.eof()) {
            c = ifs.get();
            if (c == ':') {
                ifs.unget();
                break;
            } else if (isspace(c)) {
                break;
            }

            key[cnt++] = c;

            if (cnt == kMaxKeyLength) {
                std::cerr << "[ERROR] too long key is found !!" << std::endl;
                std::abort();
            }
        }

        // Skip ':'
        while(!ifs.eof()) {
            c = ifs.get();
            if (c == ':') break;
            if (!isspace(c)) {
                std::cerr << "[ERROR] key cannot contain white space !!" << std::endl;
                std::abort();
            }
        }

        // Cull backward white spaces
        key[cnt] = '\0';
        return std::string(key);
    }

    std::string YamlParser::getNextValue(std::ifstream& ifs) const {
        skipSpace(ifs);

        int cnt = 0;
        char c;
        char val[kMaxValueLength];
        while (!ifs.eof()) {
            c = ifs.get();
            if (c == '\r') {
                char cc = ifs.get();
                if (cc == '\n') {
                    // Windows (CRLF)
                    break;
                } else {
                    // Mac (CR)
                    ifs.unget();
                    break;
                }
            } else if (c == '\n') {
                // Linux (LF)
                break;
            } else {
                val[cnt++] = c;
            }

            if (cnt == kMaxValueLength) {
                std::cerr << "[ERROR] too long value is detected !!" << std::endl;
                std::abort();
            }
        }
        // Cull backward white spaces
        while (isspace(val[--cnt])) ;
        val[cnt + 1] = '\0';
        return std::string(val);
    }

    std::string YamlParser::getNextValueIfExist(std::ifstream& ifs) const {
        skipLine(ifs);
        return "";
    }

    std::string YamlParser::getNextComment(std::ifstream& ifs) const {
        skipSpace(ifs);

        int cnt = 0;
        char c;
        char cmt[kMaxCommentLength];
        while (!ifs.eof()) {
            c = ifs.get();
            if (c == '\r') {
                char cc = ifs.get();
                if (cc == '\n') {
                    // Windows (CRLF)
                    break;
                } else {
                    // Mac (CR)
                    ifs.unget();
                    break;
                }
            } else if (c == '\n') {
                // Linux (LF)
                break;
            } else {
                cmt[cnt++] = c;         
            }

            if (cnt == kMaxCommentLength) {
                std::cerr << "[ERROR] too long comment is detected !!" << std::endl;
                std::abort();
            }
        }

        // Remove backward white spaces
        while (isspace(cmt[cnt--])) ;
        cmt[cnt + 1] = '\0';
        return std::string(cmt);
    }

}  // namespace spica
