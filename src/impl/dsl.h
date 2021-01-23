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
    
    rruleList->on([this](auto& m, bool begin) {
            if (!begin) {
                auto& ds = m.global_data();
                for (int i = 0; i < ds.size(); ++i) {
                    DSLNode* r = *ds.get<DSLNode*>(i);
                    DSLRule* rule = (DSLRule*)r;
                    this->root->rules.push_back(rule);
                }
            }
        });

    rid->on([this](auto& m, bool begin) {
        if(!begin){
            auto& d = m.global_data();
            DSLStr* node = new DSLStr(this);
            node->str = m.str();
            d.push<DSLNode*>(node);
        }
    });
    
    rall->on([this](KParser::Match& m, bool begin) {
        auto& d = m.global_data();
        auto sz = d.size();
        if(begin){
            auto node = new DSLAll(this);
            this->matchMap[&m] = sz;
            d.push<DSLNode*>(node);
        } else {
            size_t from = this->matchMap[&m];
            DSLAll* allNode = (DSLAll*)(*d.get<DSLNode*>(from));
            for (int i = from+1; i < sz; ++i) {
                DSLNode* n = *d.get<DSLNode*>(i);
                allNode->nodes.push_back(n);
            }
            for (int i = sz - 1; i > from; --i) {
                d.pop_any();
            }
        }
    });

    rany->on([this](KParser::Match& m, bool begin) {
        auto& d = m.global_data();
        auto sz = d.size();
        if (begin) {
            auto node = new DSLAny(this);
            this->matchMap[&m] = sz;
            d.push<DSLNode*>(node);
        }
        else {
            size_t from = this->matchMap[&m];
            DSLAny* allNode = (DSLAny*)(*d.get<DSLNode*>(from));
            for (int i = from + 1; i < sz; ++i) {
                DSLNode* n = *d.get<DSLNode*>(i);
                allNode->nodes.push_back(n);
            }
            for (int i = sz - 1; i > from; --i) {
                d.pop_any();
            }
        }
        });

    rrule->on([this](KParser::Match& m, bool begin) {
        auto& d = m.global_data();
        auto sz = d.size();
        if (begin) {
            auto node = new DSLRule(this);
            this->matchMap[&m] = sz;
            d.push<DSLNode*>(node);
        }
        else {
            size_t from = this->matchMap[&m];
            DSLRule* ruleNode = (DSLRule*)(*d.get<DSLNode*>(from));
            DSLStr* n = (DSLStr*)*d.get<DSLNode*>(from+1);
            ruleNode->ruleName = n->str;
            DSLNode* n1 = *d.get<DSLNode*>(from + 2);
            ruleNode->ruleNode = n1;
            d.pop_any();
            d.pop_any();
        }
    });

    //rrule->on([this](auto& m) {
    //    auto& d = m.global_data();
    //    auto sz = d.size();
    //    auto node = new DSLRule(this);
    //    std::optional<DSLNode**> n = d.get<DSLNode*>(i);
    //    node->nodes.push_back(*n.value());
    //    });
        
}

bool DSLContext::parse(std::string strRuleList) {
    auto m = rruleList->parse(strRuleList);
    if (m == nullptr) {
        return false;
    }
    auto& data = m->global_data();
    return true;
}











using namespace KParser;
class DSLParser : public KParser::Parser {
public: 
    DSLParser() {
        _id_ = identifier();
        _number_ = number_();
        _regex_ = all(str("re`"), till(str("`")));
        _term_ = any();
        _all_ = many1(_term_);
        _any_ = list(_all_, str("|"));
        _term_->add(_regex_, str_(), _id_, _number_);
        _group_ = all(str("("), _any_, str(")"));
        _term_->appendChild(_group_);
        _rule_ = all(_id_, str("="), _any_, str(";"));
        _root_ = many1(_rule_);
    }
    Rule* _group_;
    Rule* _root_;
    
    Rule* _id_;
    Rule* id_() {
        // <id> // = 
        return identifier()->on([](auto& m, bool begin) {

        });
    }

    Rule* _number_;
    Rule* number_() {
        return any(float_(), integer_());
    }

    Rule* _rule_;

    Rule* _str_;
    Rule* str_() {
        // `...
        return pred([this](const char* b, const char* e, const char*& cb, const char*& ce, const char*& me)->void {
            const char* idx = b;
            if (b == e) {
                cb = nullptr;
                ce = nullptr;
                me = nullptr;
                return;
            }
            if (*idx++ != '`') {
                cb = nullptr;
                ce = nullptr;
                me = nullptr;
                return;
            }
            cb = idx;
            while (idx != e) {
                if (*idx++ == '`') {
                    ce = idx-1;
                    me = idx;
                    return;
                }
            }
            cb = nullptr;
            ce = nullptr;
            me = nullptr;
            });
    }

    Rule* _recommend_;
    Rule* recommend_() {
        /* ... */
        return all(str("/*"), till(str("*/")));
    }

    Rule* _regex_;
    Rule* regex_() {
        /* ... */
        return all(str("re`"), till(str("`")));
    }

    Rule* _term_;
    
    Rule* _exp_;
    Rule* exp_() {
        // term_* | term_? | and_exp | or_exp
        return nullptr;
    }

    Rule* _many_;
    Rule* many_() {
        // term_* 
        return nullptr;
    }
    
    Rule* _optional_;
    Rule* optional_() {
        // term_? 
        return nullptr;
    }

    Rule* _all_;
    Rule* _any_;

};

