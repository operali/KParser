// author: operali
// desc: interface

#pragma once
#include "./detail/common.h"
#include "./detail/error.h"


namespace KLib42 {
    using IT = std::vector<KAny>::iterator;
    struct Match : private KObject {
        // length of this match
        virtual KUSIZE length() = 0;
        virtual KUSIZE location() = 0;
        // text of this match
        virtual std::string str() = 0;
        virtual std::string prefix() = 0;
        virtual std::string suffix() = 0;

        // captures in this match
        virtual KAny* captureAny(KUSIZE i) = 0;
        virtual size_t captureSize() = 0;
        template<typename T>
        inline T* capture(KUSIZE i) {
            return captureAny(i)->get<T>();
        }
    };

    struct Rule;
    struct ParserImpl;
    using CustomT = std::function<const char* (const char* begin, const char* end)>;
    using CaptureT = std::function<KAny(Match& m, IT arg, IT noarg)>;
    class Parser : private KObject {
        ParserImpl* impl;
    public:
        Parser(KUSIZE lookback = 42, bool skipBlanks = true, CustomT skipRule = nullptr);

        // some rules like comments can be skipped
        void setSkippedRule(CustomT&& r);
        KShared<KError> getLastError();
        
        void enableTrace(bool trace);
        std::string getDebugInfo();
        KShared<ISource> getSource();
        
        // epsilon
        Rule* none();
        // match string excatly
        Rule* str(std::string&& str);
        // match Pattern1 then Pattern2... then PatternN
        Rule* all();
        // match Pattern1 or Pattern2... or PatternN
        Rule* any();
        // match nothing, but cut branch
        Rule* cut();
        // ! pattern
        Rule* not_(Rule* node);
        // one char
        Rule* one();
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

        // match to end of file/text
        Rule* eof();

        // match c identifier/variable 
        Rule* identifier();
        // match c integer
        Rule* integer_();
        // match c float
        Rule* float_();
        Rule* blank();
        Rule* noblank();
        // user rule
        Rule* custom(CustomT p);
        // match Pattern1 or Pattern2... or PatternN
        template<typename ...TS>
        Rule* any(TS ...nodes);
        
        // match Pattern1 + Pattern2... + PatternN
        template<typename ...TS>
        Rule* all(TS ...nodes);

        ~Parser() override;
    };

    struct EasyParserImpl;
    struct  EasyParser  {
        // EBNF rules for parser
        void prepareRules(const char* strRule);
        // constance ID in rules, for example, NUM, ID, MYCONS...
        void prepareConstant(const std::string& idName, CustomT&& p);
        // skip rules when parsing, for example, comments of language: /*...*/, #
        void prepareSkippedRule(CustomT&& pred);
        // when matched, iterator matching tree, capture when reduce to this rule
        void prepareCapture(const char* ruleName, std::function<KAny(Match& m, IT arg, IT noarg)>&& eval);
        // after prepare...， you should compile
        bool compile();
        // after built, you can parse according to special rule 
        KUnique<Match> parse(const char* ruleName, const std::string& toParse);
        // get last error, when parsing fail or compile fail
        KShared<KError> getLastError();
        
        EasyParser();
        ~EasyParser();
    private:
        EasyParserImpl* impl;
    };

    struct Rule : private KObject {
        virtual KUnique<Match> parse(const std::string& text) = 0;
        virtual Rule* visit(std::function<void(Match&, bool)>&& handle) = 0;
        virtual Rule* eval(std::function<KAny(Match& m, IT arg, IT noarg)>&& eval) = 0;
        virtual std::string toString() = 0;
        virtual void appendChild(Rule* r) = 0;
        virtual Parser* host() = 0;
        virtual void setInfo(KAny info) = 0;
        virtual KAny& getInfo() = 0;

        template<typename T, typename ...TS>
        void add(T node, TS ...nodes) {
            this->add(node);
            this->add(nodes...);
        }

        template<typename T>
        void add(T node) {

            // to make sure static assert is valid
#if __cpp_static_assert > 201400
            static_assert(std::is_convertible<T, Rule*>::value);
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
