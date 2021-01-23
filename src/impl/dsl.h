#include "../KParser.h"
#include <vector>

struct DSLNode;
struct DSLContext : public KParser::Parser {
    std::unordered_map <std::string, DSLNode*> nodeMap;

    KParser::Rule* rid;
    KParser::Rule* rany;
    KParser::Rule* rall;
    KParser::Rule* rrule;
    KParser::Rule* rruleList;
    KParser::Rule* ritem;

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

    DSLAny(DSLContext* ctx) :DSLNode(ctx) {

    }
};

DSLContext::DSLContext() {
    rid = identifier();
    rrule = all();
    rruleList = all();
    ritem = any();
    rall = many1(ritem);
    rany = list(any(ritem, rall), str("|"));
    ritem->add(rid);// TODO more
    rrule = all(rid, str("="), any(ritem, rall, rany), str(";"));
    rruleList = many1(rrule);

    rid->on([=](auto& m, bool begin) {
        auto& d = m.global_data();
        auto node = new DSLStr(this);
        node->str = m.str();
        d.push<DSLNode*>((DSLNode*)node);
        });

    rall->on([=](auto& m, bool begin) {
        auto& d = m.global_data();
        auto sz = d.size();
        auto node = new DSLAll(this);
        for (int i = 0; i < sz; ++i) {
            std::optional<DSLNode**> n = d.get<DSLNode*>(i);
            node->nodes.push_back(*n.value());
        }
        d.push<DSLNode*>((DSLNode*)node);
    });

    //rrule->on([=](auto& m) {
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

