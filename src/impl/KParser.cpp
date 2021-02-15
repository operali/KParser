#include <string>
#include <sstream>
#include <iostream>
// for memcpy
#include <algorithm>

#include <cstdarg>
#include "impl.h"
#include <regex>
#include "dsl.h"

namespace KLib42 {
    KUSIZE KObject::count = 0;
#ifdef KOBJECT_DEBUG
    std::vector<KObject*> KObject::all;
#endif

    Parser::~Parser() {
        delete impl;
    }

    KShared<KError> Parser::errInfo() {
        return impl->parseErrInfo.clone();
        /*std::stringstream ss;
        ss << "parse fail at (line:column)" << err.row << ":" << err.col << std::endl;
        ss << std::string(err.lineLeft, err.lineMid)
            << "^^^"
            << std::string(err.lineMid, err.lineRight) << std::endl;
        return ss.str();*/
    }

    Parser::Parser(KUSIZE lookback, bool skipBlanks) :impl(new ParserImpl(this, lookback, skipBlanks)) {
    }

    Rule* Parser::custom(PredT p) {
        return new RuleCustom(impl, p);
    }

    Rule* Parser::none() {
        return new KLib42::RuleEmpty(impl);
    }

    Rule* Parser::all() {
        return new KLib42::RuleAll(impl);
    }

    Rule* Parser::any() {
        return new KLib42::RuleAny(impl);
    }

    Rule* Parser::not_(Rule* node) {
        return new KLib42::RuleNot(impl, (RuleNode*)node);
    }

    Rule* Parser::one() {
        return new KLib42::RuleOne(impl);
    }

    Rule* Parser::str(std::string&& str) {
        return new KLib42::RuleStr(impl, std::string(str));
    }

    Rule* Parser::optional(Rule* node) {
        return any(node, none());
    }

    Rule* Parser::optional(const char* strNode) {
        return optional(this->str(strNode));
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

    Rule* Parser::many(const char* strNode) {
        return many(this->str(strNode));
    }

    // n+ = n + n*
    Rule* Parser::many1(Rule* node) {
        auto nstart = many(node);
        auto nplus = all(node, nstart);
        return nplus;
    }

    Rule* Parser::many1(const char* strNode) {
        return many1(this->str(strNode));
    }

    Rule* Parser::until(Rule* cond) {
        return new RuleUntil(this->impl, (RuleNode*)cond);
    }

    Rule* Parser::until(const char* strNode) {
        return until(this->str(strNode));
    }

    // ...(pattern)
    Rule* Parser::till(Rule* cond) {
        return new RuleTill(this->impl, (RuleNode*)cond);
    }

    Rule* Parser::till(const char* strNode) {
        return till(this->str(strNode));
    }

    Rule* Parser::list(Rule* node, Rule* dem) {
        auto first = optional(node);
        auto tailItem = any(dem, node);
        auto tail = many(tailItem);
        return all(first, tail);
    }

    Rule* Parser::list(Rule* node, const char* strDem) {
        return list(node, this->str(strDem));
    }

    Rule* Parser::eof() {
        return custom([=](const char* b, const char* e)->const char* {
            if (b == e) {
                return b;
            }
            else {
                return nullptr;
            }
            });
    }

    Rule* Parser::regex(const std::string& strRe, bool startWith) {
        try {
            std::regex re(strRe.c_str());
            return custom([=](const char* b, const char* e)->const char* {
                std::match_results<const char*> results;
                // std::cout << strRe << std::endl;
                if (std::regex_search<const char*>(b, e, results, re, std::regex_constants::match_default)) {
                    auto pos = results.position();
                    if (startWith && pos != 0) {
                        return nullptr;
                    }
                    return b + results.position() + results.length();
                }
                return nullptr;
                });
        }
        catch (const std::regex_error& ex) {
            if (ex.code() == std::regex_constants::error_collate)
                std::cerr << strRe << ": error_collate.\n";
            else if (ex.code() == std::regex_constants::error_ctype)
                std::cerr << strRe << ": error_ctype.\n";
            else if (ex.code() == std::regex_constants::error_escape)
                std::cerr << strRe << ": error_escape.\n";
            else if (ex.code() == std::regex_constants::error_backref)
                std::cerr << strRe << ": error_backref.\n";
            else if (ex.code() == std::regex_constants::error_brack)
                std::cerr << strRe << ": error_brack.\n";
            else if (ex.code() == std::regex_constants::error_paren)
                std::cerr << strRe << ": error_paren.\n";
            else if (ex.code() == std::regex_constants::error_brace)
                std::cerr << strRe << ": error_brace.\n";
            else
                std::cerr << "regex.................." << ex.what() << ex.code() << std::endl;
        }
        return nullptr;
    }

    Rule* Parser::identifier() {
        return regex("^[a-zA-Z_][a-zA-Z0-9_]*");
    }

    Rule* Parser::integer_() {
        return regex("^[-+]?\\d+");
    }

    Rule* Parser::float_() {
        return regex("^[-+]?\\d*\\.?\\d+");
    }

    Rule* Parser::blank() {
        return custom([=](const char* b, const char* e)->const char* {
            for (; std::isspace(*b) && b != e; ++b) {

            }
            return b;
            });
    }

    Rule* Parser::noblank() {
        return custom([=](const char* b, const char* e)->const char* {
            for (; (!std::isspace(*b)) && b != e; ++b) {

            }
            return b;
            });
    }

    struct EasyParserImpl : public DSLContext {
        
        EasyParserImpl():DSLContext() {

        }
    };

    EasyParser::EasyParser() {
        impl = new EasyParserImpl();
    }

    EasyParser::~EasyParser() {
        delete impl;
    }

    void EasyParser::prepareRules(const char* strRule) {
        impl->prepareRules(strRule);
    }

    bool EasyParser::build() {
        return impl->build();
    }

    void EasyParser::prepareEvaluation(const char* ruleName, std::function<KAny(Match& m, IT arg, IT noarg)> eval) {
        impl->prepareEvaluation(ruleName, eval);
    }

    KUnique<Match> EasyParser::parse(const char* ruleName, const std::string& toParse) {
        return impl->parse(ruleName, toParse);
    }
    KShared<KError> EasyParser::getLastError() {
        return impl->lastError;
    };

};
