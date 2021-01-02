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
        return pred([=](const char* b, const char* e, AnyT& val)->const char* {
            const char* c = b;
            while (c != e) {
                auto m = ((RuleNode*)node)->match(c, e - c, 0);
                std::unique_ptr<MatchR> um;
                um.reset(m);
                if (um->alter()) {
                    return c;
                }
                c++;
            }
            return nullptr;
            });
    }

    // ...(pattern)
    Rule* Parser::till(Rule* cond) {
        MatchR* rootM = nullptr;
        return all(until(cond), cond);
    }

    Rule* Parser::list(Rule* node, Rule* dem) {
        auto first = optional(node);
        auto tailItem = any(dem, node);
        auto tail = many(tailItem);
        return all(first, tail);
    }

    Rule* Parser::regex(const StrT& strRe) {
        std::regex re(strRe);
        return pred([=](const char* b, const char* e, AnyT& val)->const char* {
            std::smatch results;
            std::string toSearch(b, e);
            if (std::regex_search(toSearch, results, re)) {
                val = results.str();
                return b + results.position() + results.length();
            }
            return nullptr;
            });
    }

    Rule* Parser::identifier() {
        return regex(R"(^[a-zA-Z_][a-zA-Z0-9_]*)");
    }

    Rule* Parser::integer_() {
        return regex(R"(^[-+]?\d+)");
    }

    Rule* Parser::float_() {
        return regex(R"(^[-+]?\d*\.?\d+)");
    }
};
