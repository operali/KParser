#pragma once
#include <unordered_map>
#include <sstream>

#include "../kparser.h"
#include "./rule.h"
#include "./error.h"
#include "./text.h"

namespace KLib42 {

    struct RuleNode;
    struct MatchR;
    class Parser;
    struct ParserImpl {
        Parser* m_interface;
        // config
        bool m_skipBlank;
        CustomT m_skipRule;
        KUSIZE m_lookback;
        KUSIZE m_headMax;
        std::vector<Rule*> m_headRule;
        bool m_trace;

        // state / error 
        std::vector<KAny> m_expStk;
        std::stringstream m_ss;
        KShared<KError> m_parseErrInfo;
        std::vector<MatchR*> m_tmpObj;

        // source
        const char* m_cache;
        KUSIZE length;
        KShared<ISource> m_text;
        std::vector<RuleNode*> rules;
        
        void reset();
        void setText(const std::string& text);
        void genParseError();
        void appendTmp(MatchR* obj);
        
        ParserImpl(Parser* parser, KUSIZE lookback, bool skipBlank, CustomT skipRule);
        ~ParserImpl();
    };
    
}
