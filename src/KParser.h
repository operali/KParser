#pragma once
#include <any>
#include <functional>
#include <optional>

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

    struct DataStack : private KObject {
        template<typename T>
        void emplace(T&& t) {
            push_any(std::any(std::forward<T&&>(t)));
        }

        template<typename T>
        void push(T t) {
            push_any(std::any(t));
        }
        template<typename T>
        std::optional<T> pop() {
            try {
                return std::any_cast<T>(pop_any());
            }
            catch (const std::exception& ex) {
                printf("fail to cast %s", ex.what());
            }
            return {};
        }

        template<typename T>
        void set(T t) {
            set_any(std::any(t));
        }
        template<typename T>
        T* get(size_t i) {
            try {
                return std::any_cast<T>(&get_any(i));
            }
            catch (const std::exception& ex) {
                printf("fail to cast %s", ex.what());
            }
            return nullptr;
        }

        virtual void push_any(std::any&& t) = 0;
        virtual std::any pop_any() = 0;
        virtual std::any& get_any(size_t i) = 0;
        virtual void set_any(std::any&& t, size_t i) = 0;
        virtual void clear() = 0;
        virtual size_t size() = 0;
    };


    struct Match : private KObject {
        virtual std::string occupied_str() = 0;
        virtual size_t length() = 0;
        virtual std::string str() = 0;
        virtual std::string prefix() = 0;
        virtual std::string suffix() = 0;
        virtual DataStack& global_data() = 0;
        virtual const char* global_text() = 0;
        
        virtual void visit(std::function<void(Match&, bool)> visitor) = 0;
    };

    struct Rule : private KObject {
        virtual std::unique_ptr<Match> parse(const std::string& text) = 0;
        virtual Rule* on(std::function<void(Match&, bool)> handle) = 0;
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
    using PredT = std::function<void(const char* begin, const char* textEnd, const char*& matchBegin, const char*& matchEnd, const char*& end)>;
    class Parser : private KObject {
        ParserImpl* impl;
    public:
        Parser(size_t lookback = 20, bool skipBlanks = true);

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