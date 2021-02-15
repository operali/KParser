#pragma once

#include "../KParser.h"
#include "rule.h"
#include "./error.h"

// for memcpy
#include <algorithm>


namespace KLib42 {
    
    struct RuleNode;
    class Parser;
    struct ParserImpl {
        Parser* m_interface;
        bool m_skipBlank;
        KUSIZE m_lookback;
        KUSIZE m_headMax;
        
        char* m_cache;
        KUSIZE length;
        std::vector<RuleNode*> rules;

        /*struct ParseErr {
            KUSIZE row = 0;
            KUSIZE col = 0;
            const char* lineMid = nullptr;
            const char* lineLeft = nullptr;
            const char* lineRight = nullptr;
        };*/

        KShared<KError> parseErrInfo;
        
        std::vector<KAny> m_expStk;

        std::vector<const char*> m_ll;

        void setText(const std::string& text);

        void genParseError();

        ParserImpl(Parser* parser, KUSIZE lookback, bool skipBlank);
        ~ParserImpl();
        void reset();
    };
}
