#include <string>
#include <vector>
#include <memory>
#include <any>
#include <tuple>
#include <variant>
#include <functional>

namespace KParser {
    using StrT = std::string;
    
    template<typename T>
    using UPtrT = std::unique_ptr<T>;

    using AnyT = std::any;

    template<typename T>
    using VecT = std::vector<T>;

    using PredT = std::function<const char*(const char* start, const char* end, AnyT& val)>;

    // predeclare
    struct Parser;
    UPtrT<Parser> create();
    
    struct CLSINFO {
        virtual StrT getName() = 0;
    };

    struct RuleNode;
    StrT printRule(RuleNode* n);

    struct MatchR {
        const char* m_text;
        size_t m_textLen;
        size_t m_startPos;
        AnyT m_val;
        enum LEN : int {
            FAIL = -2,
            INIT = -1,
            SUCC = 0
        };
        int m_matchLength;
        const RuleNode* m_ruleNode;
        MatchR(const char* text, size_t length, size_t start, const RuleNode* rule);

        inline bool succ(){
            return m_matchLength >= LEN::SUCC;
        }
        
        inline size_t size() {
            return (size_t)m_matchLength;
        }

        inline void accept(size_t sz) {
            m_matchLength = sz;
        }

        StrT str();
        virtual std::any& value();
        
        template<typename T>
        T* get() {
            try {
                std::any* r = &value();
                return std::any_cast<T>(r);
            }
            catch (std::exception& _) {
                return nullptr;
            }
        }

        virtual bool alter() = 0;
        virtual void visit();
    };
    struct RuleNode {
        Parser* m_gen;
        RuleNode(Parser* gen);
        virtual ~RuleNode() = default;
        virtual CLSINFO* getCLS() = 0;
        template <typename T>
        inline T* as() {
            if (T::CLS() == getCLS()) {
                return reinterpret_cast<T*>(this);
            }
            return nullptr;
        }

        virtual MatchR* match(const char* text, size_t length, size_t start) = 0;
        
        std::function<void(MatchR*)> m_eval;
        RuleNode* on(std::function<void(MatchR*)> act) {
            m_eval = act;
            return this;
        };
        MatchR* parse(const std::string& text);
    };

    struct RuleEmpty : public RuleNode{
        RuleEmpty(Parser* gen):RuleNode(gen){
        }
        ~RuleEmpty() override = default;
        static CLSINFO* CLS();
        CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }

        MatchR* match(const char* text, size_t length, size_t start) final;
    };

    struct RuleStr : public RuleNode{
        StrT m_text;
        RuleStr(Parser* gen, StrT&& text):RuleNode(gen), m_text(text){
        }
        ~RuleStr() override = default;
        static CLSINFO* CLS();
        CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }

        MatchR* match(const char* text, size_t length, size_t start) final;
    };

    struct RulePred : public RuleNode {
        PredT pred;
        RulePred(Parser* gen, PredT pred) :RuleNode(gen), pred(pred) {
        }
        static CLSINFO* CLS();
        CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }

        MatchR* match(const char* text, size_t length, size_t start) final;
    };

    struct RuleCompound : public RuleNode{
        VecT<RuleNode*> children;

        template<typename ...TS>
        inline RuleNode* cons(TS... args) {
            children = VecT<RuleNode*>{ args... };
            return this;
        }

        template<typename ...TS>
        inline RuleCompound(Parser* gen, TS... args) :RuleNode(gen), children(VecT<RuleNode*>{ args... }) {
        }
    };

    struct RuleAll : public RuleCompound {
        static CLSINFO* CLS();
        CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }
        
        template<typename ...TS>
        inline RuleAll(Parser* g, TS... args):RuleCompound(g, args...) {
        }

        MatchR* match(const char* text, size_t length, size_t start) final;
    };

    struct RuleAny : public RuleCompound {
        static CLSINFO* CLS();
        CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }

        template<typename ...TS>
        inline RuleAny(Parser* g, TS... args) :RuleCompound(g, args...) {
        }

        MatchR* match(const char* text, size_t length, size_t start) final;
    };

    struct Parser {
        bool m_skipBlank;
        static size_t count;
        // TODO
        std::string m_toparse;
        VecT<MatchR*> matchers;
        VecT<RuleNode*> rules;
        Parser(bool skipBlank = true) :m_skipBlank(skipBlank) {
            Parser::count++;
        }
        ~Parser();
        inline RuleNode* pred(PredT p) {
            return new RulePred(this, p);
        }

        template<typename ...TS>
        inline RuleAny* any(TS... args) {
            auto r = new RuleAny(this, args...);
            return r;
        }

        template<typename ...TS>
        inline RuleAll* all(TS... args) {
            auto r = new RuleAll(this, args...);
            return r;
        }

        inline RuleNode* none() {
            return new RuleEmpty(this);
        }

        inline RuleNode* str(StrT&& str) {
            return new RuleStr(this, std::string(str));
        }

        // n* = (n + n*) | epsilon;
        RuleNode* many(RuleNode* node);
        // n+ = n + n*
        RuleNode* many1(RuleNode* node);
        // n? = n | epsilon
        inline RuleNode* optional(RuleNode* node) {
            return this->any(node, this->none());
        }
        RuleNode* until(RuleNode* cond);
        RuleNode* list(RuleNode* item, RuleNode* dem);
        RuleNode* regex(const StrT& strRe);

        RuleNode* identifier();
        RuleNode* integer_();
        RuleNode* float_();
    };
};