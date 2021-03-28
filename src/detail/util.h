// author: operali
// desc: utility functions

#pragma once
namespace KLib42 {

    inline int k_isspace(int c) { return (c == ' ' || ('\t' <= c && c <= '\r')); }
    inline int k_isdigit(int c) { return ('0' <= c && c <= '9'); }

    std::string trim(const std::string& s);

    char tanslateEscapedChar(char ch);

    // c-like string, support `'`, or `"`, for example: 'asdf\n\t\345\xff'
    bool parseCSTR(const char* buff, size_t len, std::string& ret, int& rlen);

    // js-like regex object, for example: /[0-9]*/
    bool parseRegex(const char* buff, size_t len, int& rlen);

    bool parseInteger(const char* buff, size_t len, int64_t& ret, int& rlen);

    bool parseFloat(const char* buff, size_t len, double& ret, int& rlen);

    bool parseIdentifier(const char* buff, size_t len, int& rlen);
}