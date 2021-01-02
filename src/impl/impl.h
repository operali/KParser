#pragma once

#include "../KParser.h"
#include "common.h"
#include "rule.h"

struct RuleNode;
struct Parser;
namespace KParser {
    struct ParserImpl {
        Parser* m_interface;
        bool m_skipBlank;
        size_t m_lookback;
        size_t m_headMax;
        
        std::string m_toparse;
        VecT<RuleNode*> rules;

        ParserImpl(Parser* parser, size_t lookback, bool skipBlank);
        ~ParserImpl();
        void reset();
    };
}
