#pragma once
#include "../KParser.h"
#include <vector>
#include <unordered_map>

struct DSLNode;
struct DSLRuleList;
struct DSLContext : public KParser::Parser {
    std::unordered_map <std::string, DSLNode*> nodeMap;
    std::unordered_map<KParser::Match*, size_t> matchMap;

    KParser::Rule* rid;
    KParser::Rule* rany;
    KParser::Rule* rall;
    KParser::Rule* rrule;
    KParser::Rule* rruleList;
    KParser::Rule* ritem;

    DSLRuleList* root;
    DSLContext();
    bool parse(std::string str);
};

struct DSLNode {
    static const char* CLSNAME() {
        return "abstruct";
    }
    virtual std::string cls() {
        return DSLNode::CLSNAME();
    }

    std::string name;
    std::string evtName;

    DSLContext* ctx;
    DSLNode(DSLContext* ctx):ctx(ctx) {
    }
};

struct DSLStr : public DSLNode {
    static const char* CLSNAME() {
        return "str";
    }
    virtual std::string cls() {
        return DSLStr::CLSNAME();
    }
    std::string str;
    DSLStr(DSLContext* ctx) :DSLNode(ctx) {

    }
};

struct DSLRule : public DSLNode {
    static const char* CLSNAME() {
        return "rule";
    }
    virtual std::string cls() {
        return DSLRule::CLSNAME();
    }
    std::string ruleName;
    DSLNode* ruleNode = nullptr;
    DSLRule(DSLContext* ctx) :DSLNode(ctx) {

    }
};

struct DSLRuleList : public DSLNode {
    static const char* CLSNAME() {
        return "rule";
    }
    virtual std::string cls() {
        return DSLRuleList::CLSNAME();
    }
    std::vector<DSLRule*> rules;
    DSLRuleList(DSLContext* ctx) :DSLNode(ctx) {

    }
};

struct DSLAll : public DSLNode {
    static const char* CLSNAME() {
        return "str";
    }
    virtual std::string cls() {
        return DSLAll::CLSNAME();
    }

    std::vector<DSLNode*> nodes;

    DSLAll(DSLContext* ctx) :DSLNode(ctx) {

    }
};

struct DSLAny : public DSLNode {
    static const char* CLSNAME() {
        return "any";
    }
    virtual std::string cls() {
        return DSLAny::CLSNAME();
    }
    std::vector<DSLNode*> nodes;
    DSLAny(DSLContext* ctx) :DSLNode(ctx) {

    }
};

DSLContext::DSLContext() {
    this->root = new DSLRuleList(this);

    rid = identifier();
    rrule = all();
    rruleList = all();
    ritem = any();
    rall = many1(ritem);
    rany = list(any(ritem, rall), str("|"));
    ritem->add(rid);// TODO more
    rrule = all(rid, str("="), any(ritem, rall, rany), str(";"));
    rruleList = many1(rrule);
    
    rruleList->eval([this](KParser::Match& m, KParser::IT b, KParser::IT e) {
        DSLRuleList* rl = new DSLRuleList(this);
        for (; b != e; ++b) {
            rl->rules.push_back((DSLRule*)libany::any_cast<DSLNode*>(*b));
        }
        return rl;
    });
    

    rid->eval([this](KParser::Match& m, KParser::IT b, KParser::IT e) {
        DSLStr* rl = new DSLStr(this);
        rl->str = m.str();
        return (DSLNode*)rl;
        });

    rall->eval([this](KParser::Match& m, KParser::IT b, KParser::IT e) {
        DSLAll* rl = new DSLAll(this);
        for (; b != e; ++b) {
            rl->nodes.push_back(libany::any_cast<DSLNode*>(*b));
        }
        return (DSLNode*)rl;
        });

    rany->eval([this](KParser::Match& m, KParser::IT b, KParser::IT e) {
        DSLAny* rl = new DSLAny(this);
        for (; b != e; ++b) {
            rl->nodes.push_back(libany::any_cast<DSLNode*>(*b));
        }
        return (DSLNode*)rl;
        });

    rrule->eval([this](KParser::Match& m, KParser::IT b, KParser::IT e) {
        DSLRule* rl = new DSLRule(this);
        DSLNode* nameNode = libany::any_cast<DSLNode*>(*b++);
        auto name = (DSLStr*)nameNode;
        auto node = libany::any_cast<DSLNode*>(*b);
        rl->ruleName = name->str;
        rl->ruleNode = node;
        return (DSLNode*)rl;
    });
}

bool DSLContext::parse(std::string strRuleList) {
    auto m = rruleList->parse(strRuleList);
    if (m == nullptr) {
        return false;
    }
    auto** data = libany::any_cast<DSLRuleList*>(m->capture(0));
    ;
    // EXPECT_EQ(libany::any_cast<int>(m->global_value()), 1);
    return true;
}

