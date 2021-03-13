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
        Rule* m_headRule;
        bool m_trace;

        // state / error 
        std::vector<KAny> m_expStk;
        std::stringstream m_ss;
        KShared<KError> m_parseErrInfo;

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
        void setRuleInfo(RuleNode* node, uint64_t srcId);
        
        RuleInfo* getRuleInfo(RuleNode* node);

        ParserImpl(Parser* parser, KUSIZE lookback, bool skipBlank, CustomT skipRule);
        ~ParserImpl();
    };

    struct RuleInfo : private KObject {
        std::string name;
        uint64_t srcId;
        RuleInfo(std::string name, uint64_t srcId):name(name), srcId(srcId) {
        }
    };
}
