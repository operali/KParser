// author: operali
// desc: Domain special language of ENBF

#pragma once

#include <vector>
#include <unordered_map>
#include "./impl.h"
#include "./doc.h"
namespace KLib42 {
    struct DSLNode;

    struct DSLFactory {
        std::vector<DSLNode*> nodes;
        ~DSLFactory();
    };

    struct DSLNode {
        DSLFactory* builder;
        KUnique<IRange> range;
        DSLNode(DSLFactory* builder, KUnique<IRange> range);

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
        DSLID(DSLFactory* builder, KUnique<IRange> range, std::string name, bool isPreset = false) :DSLNode(builder, range), name(name), isPreset(isPreset) {

        };

        void prepare(Parser& p) override;
    };

    struct DSLText : public DSLNode {
        std::string name;
        DSLText(DSLFactory* builder, KUnique<IRange> range, std::string name) :DSLNode(builder, range), name(name) {};
        void prepare(Parser& p) override;
    };


    struct DSLRegex : public DSLNode {
        std::string name;
        DSLRegex(DSLFactory* builder, KUnique<IRange> range, std::string name) :DSLNode(builder, range), name(name) {};
        void prepare(Parser& p) override;
    };

    struct DSLWrap : public DSLNode {
        DSLNode* node;
        DSLWrap(DSLFactory* builder, KUnique<IRange> range, DSLNode* node) :DSLNode(builder, range), node(node) {};
        void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
    };

    struct DSLMany : public DSLWrap {
        DSLMany(DSLFactory* builder, KUnique<IRange> range, DSLNode* node) :DSLWrap(builder, range, node) {};

        bool build(Parser& p) override;
    };

    struct DSLMany1 : public DSLWrap {
        DSLMany1(DSLFactory* builder, KUnique<IRange> range, DSLNode* node) :DSLWrap(builder, range, node) {};

        bool build(Parser& p) override;
    };

    struct DSLList : public DSLNode {
        DSLNode* node;
        DSLNode* dem;
        DSLList(DSLFactory* builder, KUnique<IRange> range, DSLNode* node, DSLNode* dem) :DSLNode(builder, range), node(node), dem(dem) {};

        void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
        bool build(Parser& p) override;
    };

    struct DSLTill : public DSLWrap {
        DSLTill(DSLFactory* builder, KUnique<IRange> range, DSLNode* node) :DSLWrap(builder, range, node) {};
        bool build(Parser& p) override;
    };

    struct DSLOption : public DSLWrap {
        DSLOption(DSLFactory* builder, KUnique<IRange> range, DSLNode* node) :DSLWrap(builder, range, node) {};
        bool build(Parser& p) override;
    };

    struct DSLChildren : public DSLNode {
        std::vector<DSLNode*> nodes;
        DSLChildren(DSLFactory* builder, KUnique<IRange> range) :DSLNode(builder, range) {};

        void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
    };

    struct DSLAny : public DSLChildren {
        DSLAny(DSLFactory* builder, KUnique<IRange> range) :DSLChildren(builder, range) {
        };
        void prepare(Parser& p) override;
        bool build(Parser& p) override;
    };

    struct DSLAll : public DSLChildren {
        DSLAll(DSLFactory* builder, KUnique<IRange> range) :DSLChildren(builder, range) {};
        void prepare(Parser& p) override;
        bool build(Parser& p) override;
    };

    struct DSLRule : public DSLWrap {
        std::string name;
        DSLID* id;
        std::string ruleLine;
        DSLRule(DSLFactory* builder, KUnique<IRange> range, DSLNode* node) :DSLWrap(builder, range, node) {
        }
        bool build(Parser& p) override;
    };

    struct DSLRuleList : public DSLChildren {
        DSLRuleList(DSLFactory* builder, KUnique<IRange> range) :DSLChildren(builder, range) {};
    };

    struct DSLContext : DSLFactory {
        Parser m_parser;
    private:
        std::string m_strRule;
        
        std::unordered_map <std::string, DSLNode*> idMap;
        std::unordered_map <std::string, std::function<KAny(Match& m, IT arg, IT noarg)>> handleMap;

        // debug
        KShared<KError> lastError;
        KDocument m_doc;
        std::unordered_map <std::string, DSLNode*> idMap_;
        // end debug
    public:
        
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
        KShared<KError> getLastError();
        void prepareRules(const std::string& str);
        void prepareCommentRule(PredT&& p);
        void prepareAppendConstantRule(const std::string& idName, PredT&& p);
        void prepareCapture(const std::string& evtName, std::function<KAny(Match& m, IT arg, IT noarg)> handle);
        bool build();
        KUnique<Match> parse(const std::string& ruleName, const std::string& str);
    };

    template<>
    inline std::string to_string(DSLNode*& v) {
        std::stringstream ss;
        ss << "dsl_node(";
        ss << reinterpret_cast<size_t>(v);
        ss << ")";
        return ss.str();
    }
}