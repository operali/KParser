#pragma once

#include "../KParser.h"
#include "common.h"
#include "rule.h"

// for memcpy
#include <algorithm>

namespace KParser {
    
    struct RuleNode;
    class Parser;
    struct ParserImpl {
        Parser* m_interface;
        bool m_skipBlank;
        size_t m_lookback;
        size_t m_headMax;
        
        char* m_cache;
        size_t length;
        VecT<RuleNode*> rules;

        struct ErrInfo {
            size_t lineNo = 0;
            char* breakCh = nullptr;
            char* leftCh = nullptr;
            char* rightCh = nullptr;
        };

        ErrInfo err;
        
        std::vector<libany::any> m_expStk;

        void setText(const std::string& text) {
            length = text.length();
            if (length == 0) {
                m_cache = nullptr;
            }
            else {
                m_cache = new char[length+1];
                std::copy(text.begin(), text.begin() + length, m_cache);
                m_cache[length] = '\0';
            }
        }

        ParserImpl(Parser* parser, size_t lookback, bool skipBlank);
        ~ParserImpl();
        void reset();
    };
}
