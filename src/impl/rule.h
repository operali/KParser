#pragma once
#include "common.h"
#include "impl.h"
#include <optional>
#include <vector>

namespace KParser {

    struct RuleNode;
    
    struct MatchR : public Match {
        int32_t m_startPos;
        int32_t m_matchstartPos;
        int32_t m_matchstopPos;
        int32_t m_length;
        enum LEN : int16_t {
            FAIL = -2,
            INIT = -1,
            SUCC = 0
        };
        
        RuleNode* m_ruleNode;
        MatchR(size_t start, RuleNode* rule);
        virtual ~MatchR() = default;

        inline bool succ(){
            return m_length >= LEN::SUCC;
        }
        
        inline size_t length() override {
            return (size_t)m_length;
        }

        inline void accept(size_t sz) {
            m_length = sz;
        }

        StrT occupied_str() override;
        std::string str() override;
        StrT prefix() override;
        StrT suffix() override;

        DataStack& global_data() override;
        const char* global_text() override;
        
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
        void visit(std::function<void(Match& m, bool begin)> visitor) override;
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

        virtual MatchR* match(size_t start) = 0;
        std::function<void(Match& m, bool on)> m_eval;

        std::unique_ptr<Match> parse(const std::string& text) override;

        RuleNode* on(std::function<void(Match&, bool)> act) override;
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

        MatchR* match(size_t start) final;
    };

    struct RuleStr : public RuleNode{
        char* buff;
        size_t len;
        RuleStr(ParserImpl* gen, StrT text):RuleNode(gen){
            auto ptext = text.c_str();
            len = text.length();
            if (len == 0) {
                buff = nullptr;
                return;
            }
            buff = new char[len];
            memcpy(buff, ptext, len);
        }
        ~RuleStr() {
            delete[] buff;
        }
        static CLSINFO* CLS();
        CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }

        MatchR* match(size_t start) final;
    };

    struct RulePred : public RuleNode {
        PredT pred;
        RulePred(ParserImpl* gen, PredT pred) :RuleNode(gen), pred(pred) {
        }
        static CLSINFO* CLS();
        CLSINFO* getCLS() final {
            return std::decay_t<decltype(*this)>::CLS();
        }

        MatchR* match(size_t start) final;
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
        MatchR* match(size_t start) final;
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
        MatchR* match(size_t start) final;
    };
}