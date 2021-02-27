#pragma once
namespace KLib42 {
    std::string trim(const std::string& s);

    char tanslateEscapedChar(char ch);

    bool parseCSTR(const char* buff, size_t len, std::string& ret, int& rlen);

    bool parseRegex(const char* buff, size_t len, int& rlen);

    bool parseInt(const char* buff, size_t len, int64_t& ret, int& rlen);

    bool parseFloat(const char* buff, size_t len, double& ret, int& rlen);

}