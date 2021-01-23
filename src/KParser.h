#pragma once
#include "impl/any.h"
#include <type_traits>
#include <functional>
#include <memory>
#include <vector>

namespace KParser {
    struct KObject {
        static size_t count;
        //static std::vector<KObject*> all;
        KObject() {
            //all.push_back(this);
            KObject::count++;
        }
        virtual ~KObject() {
            //all.erase(std::remove(all.begin(), all.end(), this));
            KObject::count--;
        }
    };
    using IT = std::vector<libany::any>::iterator;


    struct Match : private KObject {
        virtual std::string occupied_str() = 0;
        virtual size_t length() = 0;
        virtual std::string str() = 0;
        virtual std::string prefix() = 0;
        virtual std::string suffix() = 0;
        virtual libany::any* capture(size_t i) = 0;
    };

    struct Rule;
    struct ParserImpl;
    using PredT = std::function<void(const char* begin, const char* textEnd, const char*& matchBegin, const char*& matchEnd, const char*& end)>;
    class Parser : private KObject {
        ParserImpl* impl;
    public:
        Parser(size_t lookback = 42, bool skipBlanks = true);

        // match Pattern1 + Pattern2... + PatternN
        Rule* all();
        // epsilon
        Rule* none();
        // match to string excatly
        Rule* str(std::string&& str);
        // pattern*
        Rule* many(Rule* node);
        Rule* many(const char* strNode);
        // pattern+
        Rule* many1(Rule* node);
        Rule* many1(const char* strNode);
        // pattern?
        Rule* optional(Rule* node);
        Rule* optional(const char* strNode);
        // (...)pattern
        Rule* until(Rule* cond);
        Rule* until(const char* strNode);
        // ...(pattern)
        Rule* till(Rule* cond);
        Rule* till(const char* strNode);
        // item1 dem item2 dem...itemN // for example: 1,2,3...N
        Rule* list(Rule* item, Rule* dem);
        Rule* list(Rule*, const char* dem);
        // match regex forward
        Rule* regex(const std::string& strRe, bool startWith = true);
        // match c identifier/variable 
        Rule* identifier();
        // match c integer
        Rule* integer_();
        // match c float
        Rule* float_();
        // user rule
        Rule* pred(PredT p);

        // match Pattern1 or Pattern2... or PatternN
        template<typename ...TS>
        Rule* any(TS ...nodes);

        // match Pattern1 | Pattern2... | PatternN
        Rule* any();

        // match Pattern1 + Pattern2... + PatternN
        template<typename ...TS>
        Rule* all(TS ...nodes);

        ~Parser() override;
    };



    struct Rule : private KObject {
        virtual std::unique_ptr<Match> parse(const std::string& text) = 0;
        virtual Rule* visit(std::function<void(Match&, bool)> handle) = 0;
        virtual Rule* eval(std::function<libany::any(Match& m, IT arg, IT noarg)> eval) = 0;
        virtual std::string toString() = 0;
        virtual void appendChild(Rule* r) = 0;
        virtual Parser* host() = 0;

        template<typename T, typename ...TS>
        void add(T node, TS ...nodes) {
            this->add(node);
            this->add(nodes...);
        }

        template<typename T>
        void add(T node) {

            // to make sure static assert is valid
#if __cpp_static_assert > 201400
            static_assert(std::is_same_v<T, Rule*>);
#endif
            this->appendChild(node);
        }
    };

    template<>
    inline void Rule::add(const char* node) {
        Parser* p = this->host();
        this->appendChild(p->str(node));
    }


    // match Pattern1 or Pattern2... or PatternN
    template<typename ...TS>
    inline Rule* Parser::any(TS ...nodes) {
        auto root = any();
        root->add(nodes...);
        return root;
    }


    // match Pattern1 + Pattern2... + PatternN
    template<typename ...TS>
    inline Rule* Parser::all(TS ...nodes) {
        auto root = all();
        root->add(nodes...);
        return root;
    }
};
