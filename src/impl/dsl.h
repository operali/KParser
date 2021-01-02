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
    constexpr const char* CLSNAME() {
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
    constexpr const char* CLSNAME() {
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
    constexpr const char* CLSNAME() {
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
    constexpr const char* CLSNAME() {
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
    constexpr const char* CLSNAME() {
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
    constexpr const char* CLSNAME() {
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
    
    rruleList->eval([this](auto& m, auto b, auto e) {
        DSLRuleList* rl = new DSLRuleList(this);
        for (; b != e; ++b) {
            rl->rules.push_back((DSLRule*)std::any_cast<DSLNode*>(*b));
        }
        return rl;
    });
    

    rid->eval([this](auto& m, auto b, auto e) {
        DSLStr* rl = new DSLStr(this);
        rl->str = m.str();
        return (DSLNode*)rl;
        });

    rall->eval([this](auto& m, auto b, auto e) {
        DSLAll* rl = new DSLAll(this);
        for (; b != e; ++b) {
            rl->nodes.push_back(std::any_cast<DSLNode*>(*b));
        }
        return (DSLNode*)rl;
        });

    rany->eval([this](auto& m, auto b, auto e) {
        DSLAny* rl = new DSLAny(this);
        for (; b != e; ++b) {
            rl->nodes.push_back(std::any_cast<DSLNode*>(*b));
        }
        return (DSLNode*)rl;
        });

    rrule->eval([this](auto& m, auto b, auto e) {
        DSLRule* rl = new DSLRule(this);
        DSLNode* nameNode = std::any_cast<DSLNode*>(*b++);
        auto name = (DSLStr*)nameNode;
        auto node = std::any_cast<DSLNode*>(*b);
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
    auto* data = std::any_cast<DSLRuleList*>(m->global_value());
    ;
    // EXPECT_EQ(std::any_cast<int>(m->global_value()), 1);
    return true;
}

