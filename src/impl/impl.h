#pragma once
#include <unordered_map>
#include <sstream>

#include "../kparser.h"
#include "./rule.h"
#include "./error.h"
#include "./text.h"

namespace KLib42 {

    struct RuleNode;
    class Parser;
    struct ParserImpl {
        Parser* m_interface;
        // config
        bool m_skipBlank;
        PredT m_skipRule;
        KUSIZE m_lookback;
        KUSIZE m_headMax;
        bool m_trace;

        // state / error 
        std::vector<KAny> m_expStk;
        std::stringstream m_ss;
        KShared<KError> parseErrInfo;


        // source
        const char* m_cache;
        KUSIZE length;
        KShared<ISource> m_text;
        std::vector<RuleNode*> rules;
        std::unordered_map<RuleNode*, struct RuleInfo*> m_ruleInfoMap;

        void reset();
        void setText(const std::string& text);
        void genParseError();
        void setRuleName(RuleNode* node, const std::string& name);
        RuleInfo* getRuleInfo(RuleNode* node);

        ParserImpl(Parser* parser, KUSIZE lookback, bool skipBlank, PredT skipRule);
        ~ParserImpl();
    };

    struct RuleInfo : private KObject {
        std::string name;
        RuleInfo(const std::string& name) :name(name) {}
    };
}
