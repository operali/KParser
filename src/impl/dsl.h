#pragma once

#include <vector>
#include <unordered_map>
#include "./impl.h"

namespace KLib42 {
    struct DSLNode;
    struct DSLFactory {
        std::vector<DSLNode*> nodes;
        ~DSLFactory();
    };

    struct DSLNode {
        DSLFactory* builder;
        Match* matcher;
        DSLNode(DSLFactory* builder, Match* matcher);

        virtual ~DSLNode() {}
        bool visiting = false;
        virtual void visit(std::function<void(bool sink, DSLNode* node)> handle);
        Rule* rule = nullptr;
        bool haveBuild = false;
        virtual void prepare(Parser& p) {}
        virtual bool build(Parser& p) { return true; }
    };

    struct DSLID : public DSLNode {
        std::string name;
        bool isPreset;
        DSLID(DSLFactory* builder, Match* matcher, std::string name, bool isPreset = false) :DSLNode(builder, matcher), name(name), isPreset(isPreset) {

        };

        void prepare(Parser& p) override;
    };

    struct DSLText : public DSLNode {
        std::string name;
        DSLText(DSLFactory* builder, Match* matcher, std::string name) :DSLNode(builder, matcher), name(name) {};
        void prepare(Parser& p) override;
    };


    struct DSLRegex : public DSLNode {
        std::string name;
        DSLRegex(DSLFactory* builder, Match* matcher, std::string name) :DSLNode(builder, matcher), name(name) {};
        void prepare(Parser& p) override;
    };

    struct DSLWrap : public DSLNode {
        DSLNode* node;
        DSLWrap(DSLFactory* builder, Match* matcher, DSLNode* node) :DSLNode(builder, matcher), node(node) {};
        void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
    };

    struct DSLMany : public DSLWrap {
        DSLMany(DSLFactory* builder, Match* matcher, DSLNode* node) :DSLWrap(builder, matcher, node) {};

        bool build(Parser& p) override;
    };

    struct DSLMany1 : public DSLWrap {
        DSLMany1(DSLFactory* builder, Match* matcher, DSLNode* node) :DSLWrap(builder, matcher, node) {};

        bool build(Parser& p) override;
    };

    struct DSLList : public DSLNode {
        DSLNode* node;
        DSLNode* dem;
        DSLList(DSLFactory* builder, Match* matcher, DSLNode* node, DSLNode* dem) :DSLNode(builder, matcher), node(node), dem(dem) {};

        void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
        bool build(Parser& p) override;
    };

    struct DSLTill : public DSLWrap {
        DSLTill(DSLFactory* builder, Match* matcher, DSLNode* node) :DSLWrap(builder, matcher, node) {};
        bool build(Parser& p) override;
    };

    struct DSLOption : public DSLWrap {
        DSLOption(DSLFactory* builder, Match* matcher, DSLNode* node) :DSLWrap(builder, matcher, node) {};
        bool build(Parser& p) override;
    };

    struct DSLChildren : public DSLNode {
        std::vector<DSLNode*> nodes;
        DSLChildren(DSLFactory* builder, Match* matcher) :DSLNode(builder, matcher) {};

        void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
    };

    struct DSLAny : public DSLChildren {
        DSLAny(DSLFactory* builder, Match* matcher) :DSLChildren(builder, matcher) {
        };
        void prepare(Parser& p) override;
        bool build(Parser& p) override;
    };

    struct DSLAll : public DSLChildren {
        DSLAll(DSLFactory* builder, Match* matcher) :DSLChildren(builder, matcher) {};
        void prepare(Parser& p) override;
        bool build(Parser& p) override;
    };

    struct DSLRule : public DSLWrap {
        std::string name;
        DSLID* id;
        std::string ruleLine;
        DSLRule(DSLFactory* builder, Match* matcher, DSLNode* node) :DSLWrap(builder, matcher, node) {
        }
        bool build(Parser& p) override;
    };

    struct DSLRuleList : public DSLChildren {
        DSLRuleList(DSLFactory* builder, Match* matcher) :DSLChildren(builder, matcher) {};
    };

    struct DSLContext : DSLFactory {
        std::string m_strRule;
        Parser m_parser;
        KUnique<Match> m_topMatcher;
        KShared<KError> lastError;
        std::unordered_map <std::string, DSLNode*> idMap;
        std::unordered_map <std::string, std::function<KAny(Match& m, IT arg, IT noarg)>> handleMap;
        Rule* r_comment;
        Rule* r_id;
        Rule* r_text; // `id`
        Rule* r_regex; // r`id`
        Rule* r_item; //  id, tex, re
        Rule* r_group; //  `(` r_any `)`;
        Rule* r_option; // item?
        Rule* r_many; // item*
        Rule* r_many1; // item+
        Rule* r_till; // ...item
        Rule* r_list; // [k, x]
        Rule* r_rule; // `id` `=` any `;`
        Rule* r_any; // seq(p.all , "|");
        Rule* r_all; // p.many1(item);
        Rule* r_ruleList; // 

        DSLContext();
        void prepareRules(std::string str);
        void prepareEvaluation(const std::string& evtName, std::function<KAny(Match& m, IT arg, IT noarg)> handle);
        bool build();
        KUnique<Match> parse(const std::string& ruleName, const std::string& str);
    };
}