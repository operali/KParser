#pragma once
#include "../KParser.h"
#include <vector>
#include <unordered_map>

struct DSLNode;
struct DSLFactory {
    std::vector<DSLNode*> nodes;
    ~DSLFactory();
};

struct DSLNode {
    DSLFactory* builder;
    DSLNode(DSLFactory* builder);

    virtual ~DSLNode() {}
    bool visiting = false;
    virtual void visit(std::function<void(bool sink, DSLNode* node)> handle);
    KParser::Rule* rule = nullptr;
    bool haveBuild = false;
    virtual void prepare(KParser::Parser& p) {}
    virtual bool build(KParser::Parser& p) { return true; }
};

struct DSLID : public DSLNode {
    std::string name;
    bool is_prdefine;
    DSLID(DSLFactory* builder, std::string name, bool is_prdefine = false) :DSLNode(builder), name(name), is_prdefine(is_prdefine) {};

    void prepare(KParser::Parser& p) override;
};

struct DSLText : public DSLNode {
    std::string name;
    DSLText(DSLFactory* builder, std::string name) :DSLNode(builder), name(name) {};
    void prepare(KParser::Parser& p) override;
};


struct DSLRegex : public DSLNode {
    std::string name;
    DSLRegex(DSLFactory* builder, std::string name) :DSLNode(builder), name(name) {};
    void prepare(KParser::Parser& p) override;
};

struct DSLWrap : public DSLNode {
    DSLNode* node;
    DSLWrap(DSLFactory* builder, DSLNode* node) :DSLNode(builder), node(node) {};
    void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
};

struct DSLMany : public DSLWrap {
    DSLMany(DSLFactory* builder, DSLNode* node) :DSLWrap(builder, node) {};

    bool build(KParser::Parser& p) override;
};

struct DSLMany1 : public DSLWrap {
    DSLMany1(DSLFactory* builder, DSLNode* node) :DSLWrap(builder, node) {};

    bool build(KParser::Parser& p) override;
};

struct DSLList : public DSLNode {
    DSLNode* node;
    DSLNode* dem;
    DSLList(DSLFactory* builder, DSLNode* node, DSLNode* dem) :DSLNode(builder), node(node), dem(dem) {};

    void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
    bool build(KParser::Parser& p) override;
};

struct DSLOption : public DSLWrap {
    DSLOption(DSLFactory* builder, DSLNode* node) :DSLWrap(builder, node) {};
    bool build(KParser::Parser& p) override;
};

struct DSLChildren : public DSLNode {
    std::vector<DSLNode*> nodes;
    DSLChildren(DSLFactory* builder) :DSLNode(builder) {};

    void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
};

struct DSLAny : public DSLChildren {
    DSLAny(DSLFactory* builder) :DSLChildren(builder) {
    };
    void prepare(KParser::Parser& p) override;
    bool build(KParser::Parser& p) override;
};

struct DSLAll : public DSLChildren {
    DSLAll(DSLFactory* builder) :DSLChildren(builder) {};
    void prepare(KParser::Parser& p) override;
    bool build(KParser::Parser& p) override;
};

struct DSLRule : public DSLWrap {
    std::string name;
    std::string evtName;
    std::string ruleLine;
    DSLRule(DSLFactory* builder, DSLNode* node) :DSLWrap(builder, node) {}
    bool build(KParser::Parser& p) override;
};

struct DSLRuleList : public DSLChildren {
    DSLRuleList(DSLFactory* builder) :DSLChildren(builder) {};
};



struct DSLContext : DSLFactory {
    KParser::Parser m_parser;
    std::string lastError;
    std::unordered_map <std::string, DSLNode*> idMap;
    std::unordered_map <std::string, std::function<libany::any(KParser::Match& m, KParser::IT arg, KParser::IT noarg)>> handleMap;

    KParser::Rule* r_id;
    KParser::Rule* r_text; // `id`
    KParser::Rule* r_regex; // r`id`
    KParser::Rule* r_item; //  id, tex, re
    KParser::Rule* r_group; //  `(` r_any `)`;
    KParser::Rule* r_option; // item?
    KParser::Rule* r_many; // item*
    KParser::Rule* r_many1; // item+
    KParser::Rule* r_list; // [k, x]
    KParser::Rule* r_rule; // `id` `=` any `;`
    KParser::Rule* r_any; // seq(p.all , "|");
    KParser::Rule* r_all; // p.many1(item);
    KParser::Rule* r_ruleList; // 

    DSLContext();
    bool ruleOf(std::string str);
    void bind(const std::string& evtName, std::function<libany::any(KParser::Match& m, KParser::IT arg, KParser::IT noarg)> handle);

    std::unique_ptr<KParser::Match> parse(const std::string& ruleName, const std::string& str);
};

