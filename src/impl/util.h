#pragma once
namespace KParser {
    std::string trim(const std::string& s);

    char tanslateEscapedChar(char ch);

    bool parseCSTR(const char* buff, size_t len, std::string& ret, int& rlen);

    bool parseRegex(const char* buff, size_t len, int& rlen);

}