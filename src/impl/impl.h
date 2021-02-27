#pragma once

#include "../kparser.h"
#include "./rule.h"
#include "./error.h"
#include "./text.h"

namespace KLib42 {
    
    struct RuleNode;
    class Parser;
    struct ParserImpl {
        Parser* m_interface;
        bool m_skipBlank;
        KUSIZE m_lookback;
        KUSIZE m_headMax;
        
        const char* m_cache;
        KUSIZE length;
        KShared<ISource> m_text;

        std::vector<RuleNode*> rules;

        KShared<KError> parseErrInfo;
        
        std::vector<KAny> m_expStk;

        void setText(const std::string& text);

        void genParseError();

        ParserImpl(Parser* parser, KUSIZE lookback, bool skipBlank);
        ~ParserImpl();
        void reset();
    };
}
