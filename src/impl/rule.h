#pragma once
#include "common.h"
#include "impl.h"
#include <vector>
#include "any.h"

namespace KParser {

    struct RuleNode;
    

    struct MatchR;
    struct StepInT {
        bool res;
        MatchR* mr;
    };

    struct MatchR : public Match {
        int32_t m_startPos;
        int32_t m_length;

        enum LEN : int32_t {
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

        libany::any* capture(size_t i) override;
        
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

        virtual MatchR* match(size_t start) = 0;
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
            std::copy(ptext, ptext + len, buff);
        }
        ~RuleStr() {
            delete[] buff;
        }
        MatchR* match(size_t start) final;
    };

    struct RuleCustom : public RuleNode {
        PredT pred;
        RuleCustom(ParserImpl* gen, PredT pred) :RuleNode(gen), pred(pred) {
        }
        MatchR* match(size_t start) final;
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
        MatchR* match(size_t start) final;
    };

    struct RuleAny : public RuleCompound {
        RuleAny(ParserImpl* gen) :RuleCompound(gen) {};
        MatchR* match(size_t start) final;
    };
}