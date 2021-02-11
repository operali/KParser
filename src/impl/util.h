#pragma once

namespace KParser {
    std::string parseCSTR(const char* buff, size_t len, int& rlen) {
        std::string ret;
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
                else if (ch == '/') {
                    // regex
                    break;
                }
                else if (ch == '?') {
                    // parser op
                    break;
                }
                else if (ch == '+') {
                    // parser op
                    break;
                }
                else if (ch == '*') {
                    // parser op
                    break;
                }
                else if (ch == ' ') {
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
                switch (ch) {
                case 'a':
                    ret.push_back('\a');
                    break;
                case 'b':
                    ret.push_back('\b');
                    break;
                case 'e':
                    ret.push_back('\e');
                    break;
                case 'f':
                    ret.push_back('\f');
                    break;
                case 'n':
                    ret.push_back('\n');
                    break;
                case 'r':
                    ret.push_back('\r');
                    break;
                case 't':
                    ret.push_back('\v');
                    break;
                case 'v':
                    ret.push_back('\v');
                    break;
                case '\\':
                    ret.push_back('\\');
                    break;
                case '\'':
                    ret.push_back('\'');
                    break;
                case 'x':
                    st = State::escaping_hex1;
                    ech = 0;
                    break;
                case '?':
                    ret.push_back('\?');
                    break;
                default:
                    if (ch >= '0' && ch <= '7') {
                        st = State::escaping_oct1;
                        i--;
                        ech = 0;
                    }
                    else {
                        ret.push_back(ch);
                    };
                }
            }
            else {
                std::cerr << "unknown state of " << (int)st << std::endl;
            }
        }
        rlen = i;
        return ret;
    }
}