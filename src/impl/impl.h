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

        struct ParseErr {
            int row = 0;
            int col = 0;
            const char* lineMid = nullptr;
            const char* lineLeft = nullptr;
            const char* lineRight = nullptr;
        };

        ParseErr parseErrInfo;
        
        std::vector<libany::any> m_expStk;

        std::vector<const char*> m_ll;

        void setText(const std::string& text);

        void genParseError();

        ParserImpl(Parser* parser, size_t lookback, bool skipBlank);
        ~ParserImpl();
        void reset();
    };
}
