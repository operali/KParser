#pragma once
#include <any>
#include <functional>

namespace KParser {
    struct KObject {
        static size_t count;
        static std::vector<KObject*> all;
        KObject() {
            //all.push_back(this);
            KObject::count++;
        }
        virtual ~KObject() {
            //all.erase(std::remove(all.begin(), all.end(), this));
            KObject::count--;
        }
    };

    struct Match : private KObject {
        virtual std::string str() = 0;
        virtual std::any& value() = 0;
        virtual void visit(std::function<void(Match* m)> visitor) = 0;
        virtual ~Match() = default;

        template<typename T>
        T* get() {
            try {
                std::any* r = &value();
                return std::any_cast<T>(r);
            }
            catch (const std::exception& _) {
                return nullptr;
            }
        }
    };

    struct Rule : private KObject {
        virtual std::unique_ptr<Match> parse(const std::string& text) = 0;
        virtual Rule* on(std::function<void(Match*)> handle) = 0;
        virtual std::string toString() = 0;
        virtual void appendChild(Rule* r) = 0;

        template<typename T, typename ...TS>
        void add(T node, TS ...nodes) {
            appendChild(node);
            add(nodes...);
        }

        template<typename T>
        void add(T node) {
            appendChild(node);
        }
    };


    struct ParserImpl;
    class Parser : private KObject {
        ParserImpl* impl;
        // match Pattern1 or Pattern2... or PatternN
    public:
        Parser(bool skipBlanks = true);


        // match Pattern1 + Pattern2... + PatternN
        Rule* all();
        // epsilon
        Rule* none();
        // match to string excatly
        Rule* str(std::string&& str);
        // pattern*
        Rule* many(Rule* node);
        // pattern+
        Rule* many1(Rule* node);
        // pattern?
        Rule* optional(Rule* node);
        // (...)pattern
        Rule* until(Rule* cond);
        // ...(pattern)
        Rule* till(Rule* cond);
        // item1 dem item2 dem...itemN // for example: 1,2,3...N
        Rule* list(Rule* item, Rule* dem);
        // match regex forward
        Rule* regex(const std::string& strRe);
        // match c identifier/variable 
        Rule* identifier();
        // match c integer
        Rule* integer_();
        // match c float
        Rule* float_();
        // user rule
        Rule* pred(std::function<const char* (const char* start, const char* end, std::any& matchVal)> p);


        // match Pattern1 or Pattern2... or PatternN
        template<typename ...TS>
        Rule* any(TS ...nodes) {
            auto root = any();
            root->add(nodes...);
            return root;
        }
        // match Pattern1 | Pattern2... | PatternN
        Rule* any();

        // match Pattern1 + Pattern2... + PatternN
        template<typename ...TS>
        Rule* all(TS ...nodes) {
            auto root = all();
            root->add(nodes...);
            return root;
        }
        

        ~Parser() override;
    };
};