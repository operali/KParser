// author: operali
// desc: construct rules to parse

#pragma once
#include "common.h"
#include "impl.h"
#include <vector>
#include <bitset>
#include "any.h"

namespace KLib42 {

    struct RuleNode;
    
    struct MatchR;
    struct StepInT {
        // cannot be a object* for 0x2/0x3
        MatchR* const TRUE = (MatchR*)0x2;
        MatchR* const FALSE = (MatchR*)0x3;
        MatchR* mr;
        StepInT(bool res):mr(res? TRUE : FALSE) {}
        StepInT(MatchR* mr):mr(mr) {}
        inline bool isBoolean() {
            return mr == TRUE || mr == FALSE;
        }
        inline bool booleanValue() {
            return mr == TRUE;
        }
    };

    struct MatchR : public Match {
        KUSIZE m_startPos;
        KSIZE m_length;
        
        enum LEN : KSIZE {
            FAIL = -2,
            INIT = -1,
            CERT = -3,
            SUCC = 0
        };
        

        RuleNode* m_ruleNode;
        MatchR(KUSIZE start, RuleNode* rule);
        
        KUSIZE length() override {
            return (KUSIZE)m_length;
        }

        KUSIZE location() override {
            return m_startPos;
        }

        std::string str() override;
        std::string prefix() override;
        std::string suffix() override;

        KAny* captureAny(KUSIZE i) override;
        size_t captureSize() override;
        
        virtual StepInT stepIn() {
            return StepInT(true);
        };

        virtual void stepOut(MatchR* r) {
        };

        virtual bool alter();
        virtual void visit(std::function<void(Match& m, bool isSink)> visitor);
        virtual MatchR* visitStep();

        virtual void release();
    };

    struct RuleState {
        std::vector<int> _pos;
        int getPos() {
            if (_pos.empty()) {
                return -1;
            }
            else {
                return _pos.back();
            }
        }

        void pushPos(int pos) {
            _pos.push_back(pos);
        }

        void popPos() {
            _pos.pop_back();
        }
    };

    struct RuleNode : public Rule {
        ParserImpl* m_gen;
        
        RuleState m_state;

        RuleNode(ParserImpl* gen);
        virtual ~RuleNode() = default;
        
        virtual MatchR* match(KUSIZE start) = 0;
        std::function<void(Match& m, bool on)> m_visitHandle;
        std::function<KAny(Match& m, IT arg, IT noarg)> m_evalHandle;

        KUnique<Match> parse(const std::string& text) override;

        RuleNode* visit(std::function<void(Match&, bool)>&& act) override;
        RuleNode* eval(std::function<KAny(Match& m, IT arg, IT noarg)>&& eval) override;
        void appendChild(Rule* r) override;
        std::string toString() override;
        Parser* host() override;

        KAny m_info;
        void setInfo(KAny info) override;
        KAny& getInfo() override;
    };

    struct RuleEmpty : public RuleNode{
        RuleEmpty(ParserImpl* gen):RuleNode(gen){
        }
        ~RuleEmpty() override = default;
        MatchR* match(KUSIZE start) final;
    };

    struct RuleStr : public RuleNode{
        char* buff;
        KUSIZE len;
        RuleStr(ParserImpl* gen, const std::string& text):RuleNode(gen){
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
        MatchR* match(KUSIZE start) final;
    };

    struct RuleCustom : public RuleNode {
        CustomT pred;
        RuleCustom(ParserImpl* gen, CustomT pred) :RuleNode(gen), pred(pred) {
        }
        MatchR* match(KUSIZE start) final;
    };

    struct RuleCompound : public RuleNode {
        std::vector<RuleNode*> children;
        void appendChild(Rule* r) override {
            children.push_back((RuleNode*)r);
        }
        RuleCompound(ParserImpl* gen) :RuleNode(gen) {}
    };

    struct RuleCut : public RuleNode {
        RuleCut(ParserImpl* gen) :RuleNode(gen) {};
        MatchR* match(KUSIZE start) final;
    };

    struct RuleAll : public RuleCompound {
        RuleAll(ParserImpl* gen) :RuleCompound(gen) {};
        MatchR* match(KUSIZE start) final;
    };

    struct RuleAny : public RuleCompound {
        RuleAny(ParserImpl* gen) :RuleCompound(gen) {};
        MatchR* match(KUSIZE start) final;
    };

    struct RuleTill : public RuleNode {
        RuleNode* m_cond;
        RuleTill(ParserImpl* gen, RuleNode* cond) :RuleNode(gen), m_cond(cond) {};
        MatchR* match(KUSIZE start) final;
    };

    struct RuleUntil : public RuleNode {
        RuleNode* m_cond;
        RuleUntil(ParserImpl* gen, RuleNode* cond) :RuleNode(gen), m_cond(cond) {};
        MatchR* match(KUSIZE start) final;
    };

    struct RuleNot : public RuleNode {
        RuleNode* m_cond;
        RuleNot(ParserImpl* gen, RuleNode* cond) :RuleNode(gen), m_cond(cond) {};
        MatchR* match(KUSIZE start) final;
    };

    struct RuleOne : public RuleNode {
        RuleOne(ParserImpl* gen) :RuleNode(gen) {};
        MatchR* match(KUSIZE start) final;
    };
}