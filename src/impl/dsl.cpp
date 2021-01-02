#include "../KParser.h"


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
        return identifier()->visit([](auto& m, bool capture) {

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

