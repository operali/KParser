#include <string>
#include <sstream>
#include <iostream>
#include <regex>
#include <cstdarg>
#include "impl.h"


namespace KParser {
    size_t KObject::count = 0;
    std::vector<KObject*> KObject::all;

    Parser::~Parser() {
        delete impl;
    }

    Parser::Parser(size_t lookback, bool skipBlanks) {
        impl = new ParserImpl(this, lookback, skipBlanks);
    }

    Rule* Parser::pred(PredT p) {
        return new RulePred(impl, p);
    }

    Rule* Parser::none() {
        return new KParser::RuleEmpty(impl);
    }

    Rule* Parser::all() {
        return new KParser::RuleAll(impl);
    }

    Rule* Parser::any() {
        return new KParser::RuleAny(impl);
    }


    Rule* Parser::str(StrT&& str) {
        return new KParser::RuleStr(impl, std::string(str));
    }

    Rule* Parser::optional(Rule* node) {
        return any(node, none());
    }

    Rule* Parser::many(Rule* node) {
        auto kstart = any();
        auto epsilon = none();
        auto k_kstart = all();
        k_kstart->appendChild((RuleNode*)node);
        k_kstart->appendChild(kstart);
        kstart->appendChild(k_kstart);
        kstart->appendChild((RuleNode*)epsilon);
        return kstart;
    }

    // n+ = n + n*
    Rule* Parser::many1(Rule* node) {
        auto nstart = many(node);
        auto nplus = all(node, nstart);
        return nplus;
    }

    Rule* Parser::until(Rule* node) {
        return pred([=](const char* b, const char* e, const char*& cb, const char*& ce, const char*& me)->void {
            const char* c = b;
            auto n = ((RuleNode*)node);
            auto psrc = n->m_gen->m_cache;
            while (c != e) {
                auto m = n->match(c-psrc);
                std::unique_ptr<MatchR> um;
                um.reset(m);
                if (um->alter()) {
                    cb = b;
                    ce = c;
                    me = c;
                    return;
                }
                c++;
            }
            cb = nullptr;
            ce = nullptr;
            me = nullptr;
            return;
            });
    }

    // ...(pattern)
    Rule* Parser::till(Rule* cond) {
        return all(until(cond), cond);
    }

    Rule* Parser::list(Rule* node, Rule* dem) {
        auto first = optional(node);
        auto tailItem = any(dem, node);
        auto tail = many(tailItem);
        return all(first, tail);
    }

    Rule* Parser::regex(const StrT& strRe, bool startWith) {
        std::regex re(strRe);
        
        return pred([=](const char* b, const char* e, const char*& cb, const char*& ce, const char*& me)->void {
            std::match_results<const char*> results;
            if (std::regex_search<const char*>(b, e, results, re)) {
                auto pos = results.position();
                if (startWith) {
                    if (pos != 0) {
                        cb = nullptr;
                        ce = nullptr;
                        me = nullptr;
                        return;
                    }
                }
                if (results.size() > 1) {
                    auto c = results[1];
                    cb = c.first;
                    ce = c.second;
                    me = b + results.position() + results.length();
                }
                else {
                    cb = b + results.position();
                    ce = cb + results.length();
                    me = ce;
                }
                return;
            }
            cb = nullptr;
            ce = nullptr;
            me = nullptr;
            return;
            });
    }

    Rule* Parser::identifier() {
        return regex("^_?[a-zA-Z0-9]+");
            char* ob;
            char* oe;
            pred([this](const char* b, const char* e, const char*& cb, const char*& ce, const char*& me)->void {
            if (b == e) {
                cb = nullptr;
                ce = nullptr;
                me = nullptr;
            }
            auto i = b;
            auto c = *i++;
            auto isChar = [](char c) {return (c <= 'Z' && c >= 'A') || (c <= 'z' && c >= 'a'); };
            auto isNum = [](char c) {return c <= '9' && c >= '0'; };

            if (isChar(c) || c == '_') {
                if (i == e) {
                    cb = b;
                    ce = i;
                    me = i;
                    return;
                }
                while(c = *i++) {
                    if (!isChar(c) && !isNum(c)) {
                        break;
                    }
                    if (i == e) {
                        break;
                    }
                }
                cb = b;
                ce = i;
                me = i;
                return;
            }
            else {
                cb = nullptr;
                ce = nullptr;
                me = nullptr;
                return;
            }
            });
    }

    Rule* Parser::integer_() {
        return regex(R"(^[-+]?\d+)");
    }

    Rule* Parser::float_() {
        return regex(R"(^[-+]?\d*\.?\d+)");
    }
};
