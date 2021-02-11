#include <algorithm>
#include <string>
#include <iostream>
#include "util.h"

namespace KParser {
    
    std::string trim(const std::string& s)
    {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && isspace(*it))
            it++;

        std::string::const_reverse_iterator rit = s.rbegin();
        while (rit.base() != it && isspace(*rit))
            rit++;

        return std::string(it, rit.base());
    }


    bool test_keych(char ch) {
        /* 
         * unary op: ? + *
         * binary op: | ' '
         * group: ( ) [ ]
         * keyword: = ;
         * other first set: $ /
        */
        static char firstChars[] = { '?', '+', '*', '|', ' ', '(', ')', '[', ']', '=', ';', '$', '/' };
        char* begin = &firstChars[0];
        char* end = begin + sizeof(firstChars);
        auto* it = std::find(begin, end, ch);
        if (it == end) {
            return false;
        }
        return true;
    }

    char tanslate_escapech(char ch) {
        switch (ch) {
        case 'a':
            return '\a';
            break;
        case 'b':
            return ('\b');
            break;
        case 'e':
            return ('\e');
            break;
        case 'f':
            return ('\f');
            break;
        case 'n':
            return ('\n');
            break;
        case 'r':
            return ('\r');
            break;
        case 't':
            return ('\v');
            break;
        case 'v':
            return ('\v');
        default:
            return 0;
        }
    }

    bool parseCSTR(const char* buff, size_t len, std::string& ret, int& rlen) {
        enum class State : int8_t {
            read_ch,
            escaping,
            escaping_hex1,
            escaping_hex2,
            escaping_oct1,
            escaping_oct2,
            escaping_oct3,
            done,
        };
        State st = State::read_ch;
        int ech = 0;
        int i = 0;
        for (; i < len; ++i) {
            auto ch = buff[i];
            if (st == State::read_ch) {
                if (ch == '\\') {
                    st = State::escaping;
                }
                else if (test_keych(ch)) {
                    st == State::done;
                    break;
                }
                else {
                    ret.push_back(ch);
                }
            }
            else if (st == State::escaping_oct1) {
                st = State::escaping_oct2;
                if (ch >= '0' && ch <= '7') {
                    ech = (uint8_t)ech * 8 + (ch - '0');
                }
                else {
                    st = State::read_ch;
                    i--;
                }
            }
            else if (st == State::escaping_oct2) {
                st = State::escaping_oct3;
                if (ch >= '0' && ch <= '7') {
                    ech = (uint8_t)ech * 8 + (ch - '0');
                }
                else {
                    st = State::read_ch;
                    i -= 2;
                }
            }
            else if (st == State::escaping_oct3) {
                st = State::read_ch;
                if (ch >= '0' && ch <= '7') {
                    ech = (uint8_t)ech * 8 + (ch - '0');
                }
                else {
                    st = State::read_ch;
                    i -= 3;
                }
                if (ech > 0xff) {
                    st = State::read_ch;
                    i -= 3;
                }
                else {
                    ret.push_back(ech);
                }
            }
            else if (st == State::escaping_hex1) {
                st = State::escaping_hex2;
                if (ch >= '0' && ch <= '9') {
                    ech = (uint8_t)ech * 16 + (ch - '0');
                }
                else if (ch > 'a' && ch <= 'f') {
                    ech = (uint8_t)ech * 16 + 10 + (ch - 'a');
                }
                else if (ch > 'A' && ch <= 'F') {
                    ech = (uint8_t)ech * 16 + 10 + (ch - 'A');
                }
                else {
                    ech = 0;
                    st = State::read_ch;
                    i--;
                }
            }
            else if (st == State::escaping_hex2) {
                st = State::read_ch;
                if (ch >= '0' && ch <= '9') {
                    ech = (uint8_t)ech * 16 + (ch - '0');
                }
                else if (ch > 'a' && ch <= 'f') {
                    ech = (uint8_t)ech * 16 + 10 + (ch - 'a');
                }
                else if (ch > 'A' && ch <= 'F') {
                    ech = (uint8_t)ech * 16 + 10 + (ch - 'A');
                }
                else {
                    ech = 0;
                    st = State::read_ch;
                    i -= 2;
                }
                ret.push_back(ech);
            }
            else if (st == State::escaping) {
                st = State::read_ch;
                char nch = tanslate_escapech(ch);
                if (nch != 0) {
                    ret.push_back(nch);
                }
                else if(ch == 'x'){
                    st = State::escaping_hex1;
                    ech = 0;
                }
                else if (ch >= '0' && ch <= '7') {
                    st = State::escaping_oct1;
                    i--;
                    ech = 0;
                }
                else {
                    ret.push_back(ch);
                };
            }
            else {
                std::cerr << "unknown state of " << (int)st << std::endl;
            }
        }
        rlen = i;
        if (rlen == 0) {
            return false;
        }
        return true;
    }

    bool parseRegex(const char* buff, size_t len, int& rlen) {
        int i = 0;
        enum class State : int8_t {
            read_left,
            read_ch,
            escaping,
            read_done
        };
        State st = State::read_left;
        char ch = 257;
        for (; i < len; ++i) {
            auto ch = buff[i];
            if (st == State::read_left) {
                if (ch != '/') {
                    return false;
                }
                st = State::read_ch;
            }
            else if (st == State::read_ch) {
                if (ch == '\\') {
                    st = State::escaping;
                }
                else if (ch == '/') {
                    // done
                    st = State::read_done;
                    i++;
                    break;
                }
                else if (ch == '\n') {
                    // can not be multiple line
                    return false;
                }
            }
            else if (st == State::escaping) {
                // no excatly, only omit it to read state
                st = State::read_ch;
            }
        }
        rlen = i;
        if (i <= 2) {
            return false;
        }
        if (st != State::read_done) {
            return false;
        }
        return true;
    }
}