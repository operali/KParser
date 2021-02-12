#pragma once
#include "common.h"
#include "impl.h"
#include <vector>
#include "any.h"

namespace KParser {

    struct RuleNode;
    
    struct MatchR;
    struct StepInT {
        MatchR* mr;
        bool res;
        StepInT(bool res):mr(nullptr), res(res) {}
        StepInT(MatchR* mr):mr(mr), res(false) {}
    };

    struct MatchR : public Match {
        uint32_t m_startPos;
        int32_t m_length;

        enum LEN : int32_t {
            FAIL = -2,
            INIT = -1,
            SUCC = 0
        };
        
        RuleNode* m_ruleNode;
        MatchR(uint32_t start, RuleNode* rule);
        virtual ~MatchR() = default;

        inline bool succ(){
            return m_length >= LEN::SUCC;
        }
        
        inline uint32_t length() override {
            return (uint32_t)m_length;
        }

        inline void accept(uint32_t sz) {
            m_length = sz;
        }

        StrT occupied_str() override;
        std::string str() override;
        StrT prefix() override;
        StrT suffix() override;

        libany::any* capture(uint32_t i) override;
        
        std::string errInfo() final;

        virtual StepInT stepIn() {
            throw std::exception();//TODO
        };

        virtual void stepOut(MatchR* r) {
            throw std::exception();//TODO
        };

        virtual bool alter();
        virtual void visit(std::function<void(Match& m, bool capture)> visitor);
        virtual MatchR* visitStep();

        virtual void release();
    };
    struct RuleNode : public Rule {
        ParserImpl* m_gen;
        RuleNode(ParserImpl* gen);
        virtual ~RuleNode() = default;
        template <typename T>
        inline T* as() {
            return dynamic_cast<T*>(this);
        }

        virtual MatchR* match(uint32_t start) = 0;
        std::function<void(Match& m, bool on)> m_visitHandle;
        std::function<libany::any(Match& m, IT arg, IT noarg)> m_evalHandle;

        std::unique_ptr<Match> parse(const std::string& text) override;

        RuleNode* visit(std::function<void(Match&, bool)> act) override;
        RuleNode* eval(std::function<libany::any(Match& m, IT arg, IT noarg)> eval) override;
        void appendChild(Rule* r) override;
        std::string toString() override;
        Parser* host() override;
    };

    struct RuleEmpty : public RuleNode{
        RuleEmpty(ParserImpl* gen):RuleNode(gen){
        }
        ~RuleEmpty() override = default;
        MatchR* match(uint32_t start) final;
    };

    struct RuleStr : public RuleNode{
        char* buff;
        uint32_t len;
        RuleStr(ParserImpl* gen, StrT text):RuleNode(gen){
            auto ptext = text.c_str();
            len = text.length();
            if (len == 0) {
                buff = nullptr;
                return;
            }
            buff = new char[len];
            std::copy(ptext, ptext + len, buff);
        }
        ~RuleStr() {
            delete[] buff;
        }
        MatchR* match(uint32_t start) final;
    };

    struct RuleCustom : public RuleNode {
        PredT pred;
        RuleCustom(ParserImpl* gen, PredT pred) :RuleNode(gen), pred(pred) {
        }
        MatchR* match(uint32_t start) final;
    };

    struct RuleCompound : public RuleNode {
        VecT<RuleNode*> children;
        void appendChild(Rule* r) override {
            children.push_back((RuleNode*)r);
        }
        RuleCompound(ParserImpl* gen) :RuleNode(gen) {}
    };

    struct RuleAll : public RuleCompound {
        RuleAll(ParserImpl* gen) :RuleCompound(gen) {};
        MatchR* match(uint32_t start) final;
    };

    struct RuleAny : public RuleCompound {
        RuleAny(ParserImpl* gen) :RuleCompound(gen) {};
        MatchR* match(uint32_t start) final;
    };

    struct RuleTill : public RuleNode {
        RuleNode* m_cond;
        RuleTill(ParserImpl* gen, RuleNode* cond) :RuleNode(gen), m_cond(cond) {};
        MatchR* match(uint32_t start) final;
    };

    struct RuleUntil : public RuleNode {
        RuleNode* m_cond;
        RuleUntil(ParserImpl* gen, RuleNode* cond) :RuleNode(gen), m_cond(cond) {};
        MatchR* match(uint32_t start) final;
    };

    struct RuleNot : public RuleNode {
        RuleNode* m_cond;
        RuleNot(ParserImpl* gen, RuleNode* cond) :RuleNode(gen), m_cond(cond) {};
        MatchR* match(uint32_t start) final;
    };

    struct RuleOne : public RuleNode {
        RuleOne(ParserImpl* gen) :RuleNode(gen) {};
        MatchR* match(uint32_t start) final;
    };
}