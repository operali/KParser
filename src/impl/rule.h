#pragma once
#include "common.h"
#include "impl.h"
#include <optional>
#include <vector>

namespace KParser {

    struct RuleNode;
    
    struct MatchR : public Match {
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
        RuleNode* m_ruleNode;
        MatchR(const char* text, size_t length, size_t start, RuleNode* rule);
        virtual ~MatchR() = default;

        inline bool succ(){
            return m_matchLength >= LEN::SUCC;
        }
        
        inline size_t size() {
            return (size_t)m_matchLength;
        }

        inline void accept(size_t sz) {
            m_matchLength = sz;
        }

        StrT str() override;
        std::any& value() override;
        
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

        virtual std::variant<bool, MatchR*> stepIn() {
            throw std::exception("unimplemented");
        };

        virtual void stepOut(MatchR* r) {
            throw std::exception("unimplemented");
        };

        virtual bool alter();
        void visit(std::function<void(Match* m)> visitor) override;
        virtual MatchR* visitStep();

        virtual void release();
    };
    struct RuleNode : public Rule {
        ParserImpl* m_gen;
        RuleNode(ParserImpl* gen);
        virtual ~RuleNode() = default;
        virtual const CLSINFO* getCLS() = 0;
        template <typename T>
        inline T* as() {
            if (T::CLS() == getCLS()) {
                return reinterpret_cast<T*>(this);
            }
            return nullptr;
        }

        virtual MatchR* match(const char* text, size_t length, size_t start) = 0;
        std::function<void(Match*)> m_eval;

        std::unique_ptr<Match> parse(const std::string& text) override;

        RuleNode* on(std::function<void(Match*)> act) override;
        void appendChild(Rule* r) override;
        std::string toString() override;
    };

    struct RuleEmpty : public RuleNode{
        RuleEmpty(ParserImpl* gen):RuleNode(gen){
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
        RuleStr(ParserImpl* gen, StrT&& text):RuleNode(gen), m_text(text){
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
        RulePred(ParserImpl* gen, PredT pred) :RuleNode(gen), pred(pred) {
        }
        static CLSINFO* CLS();
        CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }

        MatchR* match(const char* text, size_t length, size_t start) final;
    };

    struct RuleAll : public RuleNode  {
        VecT<RuleNode*> children;
        void appendChild(Rule* r) override {
            children.push_back((RuleNode*)r);
        }
        const static CLSINFO* CLS();

        const CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }

        RuleAll(ParserImpl* gen) :RuleNode(gen) {};
        MatchR* match(const char* text, size_t length, size_t start) final;
    };

    struct RuleAny : public RuleNode  {
        VecT<RuleNode*> children;
        void appendChild(Rule* r) override {
            children.push_back((RuleNode*)r);
        }
        static CLSINFO* CLS();
        CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }
        RuleAny(ParserImpl* gen) :RuleNode(gen) {};
        MatchR* match(const char* text, size_t length, size_t start) final;
    };
}