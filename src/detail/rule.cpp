// author: operali
// desc: construct rules to parse

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "util.h"
#include "impl.h"

// #define isspace(ch) ((ch) == ' ' || (ch) == '\t' || (ch) == '\r' || (ch) == '\n' || (ch) == '\v')

namespace KLib42 {
    struct LINE {
        RuleNode* node;
        KSIZE iden;
        KSIZE id;
    };

    MatchR::MatchR(KUSIZE start, RuleNode* rule) 
        :m_startPos(start), m_ruleNode(rule), m_length(LEN::INIT) {
    }

    using LINES = std::vector<LINE>;
    static int idCount = 0;
    static void collectRuleInfo(RuleNode* n, LINES& r, int iden) {
        LINE line;
        line.node = n;
        line.iden = iden;
        line.id = -1;
        auto it = std::find_if(r.begin(), r.end(), [&](LINE& l) {
            return n == l.node;
            });
        if (it != r.end()) {
            auto& id = it->id;
            if (id == -1) {
                id = idCount++;
            }
            line.id = id;
            r.push_back(line);
            return;
        }
        r.push_back(line);
        auto* allN = dynamic_cast<RuleAll*>(n);
        if (allN) {
            for (auto c : allN->children) {
                collectRuleInfo(c, r, iden + 1);
            }
        }
        else {
            auto anyN = dynamic_cast<RuleAny*>(n);
            if (anyN) {
                for (auto c : anyN->children) {
                    collectRuleInfo(c, r, iden + 1);
                }
            }
        }
    }

    static std::string printRuleLine(RuleNode* n) {
        auto n1 = dynamic_cast<RuleAll*>(n);
        if (n1) {
            return "All";
        }
        {
            auto n1 = dynamic_cast<RuleAny*>(n);
            if (n1) {
                return "Any";
            }
        }
        {
            auto n1 = dynamic_cast<RuleStr*>(n);
            if (n1) {
                return "Str(" +std::string(n1->buff, n1->buff + n1->len) + ")";
            }
        }
        {
            auto n1 = dynamic_cast<RuleEmpty*>(n);
            if (n1) {
                return "Empty";
            }
        }
        {
            auto n1 = dynamic_cast<RuleCustom*>(n);
            if (n1) {
                return "Custom";
            }
            return "Unknown";
        }
    }

    RuleNode::RuleNode(ParserImpl* gen) :m_gen(gen) {
        gen->rules.push_back(this);
    }

    RuleNode* RuleNode::visit(std::function<void(Match&, bool)>&& act) {
        m_visitHandle = act;
        return this;
    };

    RuleNode* RuleNode::eval(std::function<KAny(Match& m, IT arg, IT noarg)>&& eval){
        m_evalHandle = eval;
        return this;
    }

    void RuleNode::appendChild(Rule* r) {
        throw std::exception();//TODO
    };

    std::string RuleNode::toString() {
        LINES lns;
        idCount = 0;
        collectRuleInfo(this, lns, 0);
        std::stringstream ss;
        for (auto& ln : lns) {
            
            for (auto i = 0; i < ln.iden; ++i) {
                ss << "  ";
            }
            if (ln.id == -1) {
                ss << printRuleLine(ln.node) << "\n";
            }
            else {
                ss << printRuleLine(ln.node) << ":" << ln.id << "\n";
            }
        }
        return ss.str();
    }

    Parser* RuleNode::host() {
        return m_gen->m_interface;
    }

    void RuleNode::setInfo(KAny info) {
        m_info = info;
    }

    KAny& RuleNode::getInfo() {
        return m_info;
    }

    KUnique<Match> RuleNode::parse(const std::string& text) {
        m_gen->reset();
        this->m_gen->setText(text);

        auto m = this->match(0);
        KUnique<Match> um;
        um.reset(m);

        std::vector<KAny>& expStk = m_gen->m_expStk;
        using IT = std::vector<KAny>::iterator;
        std::vector<KUSIZE> opStk;

        auto r = m->alter();
        if (!r) {
            return nullptr;
        }
        // m_gen->dataStk.clear();
        auto f = m->m_ruleNode->m_visitHandle;
        if (f) {
            try {
                f(*m, true);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
        opStk.push_back(expStk.size());

        m->visit([&](KLib42::Match& m, bool is_begin) {
            auto mr = (MatchR*)&m;
            auto rule = mr->m_ruleNode;
            auto f = rule->m_visitHandle;
            if (f) {
                try {
                    f(*mr, is_begin);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
            }
            if (is_begin) {
                opStk.push_back(expStk.size());
            } else {
                auto eval = mr->m_ruleNode->m_evalHandle;
                if (eval) {
                    auto from = opStk.back();
                    IT b = expStk.begin() + from;
                    try {
                        auto k = eval(*mr, b, expStk.end());
                        expStk.erase(b, expStk.end());
                        if (!k.is<nullptr_t>()) {
                            expStk.emplace_back(std::move(k));
                        }
                    }
                    catch (std::exception& e) {
                        std::cerr << e.what() << std::endl;
                    }
                }
                opStk.pop_back();
                delete mr;
            }
        });
        if (f) {
            try {
                f(*m, false);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
        auto eval = m->m_ruleNode->m_evalHandle;
        if (eval) {
            KAny res;
            auto from = opStk.back();
            IT b = expStk.begin() + from;
            try {
                auto v = eval(*m, b, expStk.end());
                v.swap(res);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            expStk.erase(b, expStk.end());
            if (!res.empty()) {
                expStk.emplace_back(std::move(res));
            }
            opStk.pop_back();
        }
        return um;
    }

    //////////////////////////////////////////////////////////////////////////
    //alter

    bool MatchR::alter() {
        std::vector<MatchR*> vec;
        MatchR* curM = this;
        curM->m_ruleNode->m_state.pushPos(this->m_startPos);
        auto* gen = m_ruleNode->m_gen;
        auto lookback = gen->m_lookback;
        auto* text = gen->m_cache;
        bool trace = gen->m_trace;

        int outputMargin = 0;

        if (trace) {
            auto* ruleNode = curM->m_ruleNode;
            auto* parser = ruleNode->m_gen;
            auto& info = ruleNode->getInfo();
            if (!info.empty()) {
                for (size_t i = 0; i < outputMargin; ++i) {
                    parser->m_ss << " ";
                }
                parser->m_ss << "on_rule: " << info.toString() << std::endl;
            }
            outputMargin++;
        }
        while (true) {
            auto st = curM->stepIn();
            if (st.isBoolean()) {
                // TODO, left recursive parsing?
                curM->m_ruleNode->m_state.popPos();
                auto succ = st.booleanValue();
                if (curM == this) {
                    if (succ) {
                        return true;
                    }
                    else {
                        gen->genParseError();
                        return false;
                    }
                }
                else {
                    MatchR* lastM = curM;
                    curM = vec.back();
                    vec.pop_back();
                    if (succ) {
                        if (trace) {
                            auto* ruleNode = lastM->m_ruleNode;
                            auto* parser = ruleNode->m_gen;
                            auto& info = ruleNode->getInfo();
                            if (!info.empty()) {
                                for (size_t i = 0; i < outputMargin; ++i) {
                                    parser->m_ss << " ";
                                }
                                parser->m_ss << "succ " << info.toString() << std::endl;
                            }

                            outputMargin--;
                        }

                        auto headLength = lastM->m_startPos + lastM->length();
                        if (headLength > gen->m_headMax) {
                            gen->m_headMax = headLength;
                            gen->m_headRule.clear();
                        }
                        curM->stepOut(lastM);
                    }
                    else {
                        if(trace) {
                            auto* ruleNode = lastM->m_ruleNode;
                            auto* parser = ruleNode->m_gen;
                            auto& info = ruleNode->getInfo();
                            if (!info.empty()) {
                                for (size_t i = 0; i < outputMargin; ++i) {
                                    parser->m_ss << " ";
                                }
                                parser->m_ss << "fail " << info.toString() << std::endl;
                            }
                            outputMargin--;
                        }

                        auto back = (int)gen->m_headMax - (int)lastM->m_startPos;
                        if (back == 0) {
                            gen->m_headRule.push_back(lastM->m_ruleNode);
                        }
                        if (back > (int)lookback) {
                            auto* node = dynamic_cast<RuleCompound*>(lastM->m_ruleNode);
                            if (node) {
                                // clear all temporary matcher
                                for (int i = vec.size() - 1; i > -1; --i) {
                                    vec[i]->release();
                                }
                                std::cerr << "lookback more than max length of " << gen->m_headMax << " chars, abandon\n";
                                gen->genParseError();
                                return false;
                            }
                        }
                        curM->stepOut(nullptr);
                    }
                }
            }
            else {
                MatchR* matcher = st.mr;
                // TODO, left recursive parsing
                if (matcher->m_ruleNode->m_state.getPos() == matcher->m_startPos) {
                    /*matcher->release();
                    delete matcher;*/
                    curM->stepOut(nullptr);
                } else {
                    // TODO, left recursive parsing
                    matcher->m_ruleNode->m_state.pushPos(matcher->m_startPos);
                    vec.push_back(curM);
                    curM = matcher;
                    if (trace) {
                        auto* ruleNode = curM->m_ruleNode;
                        auto* parser = ruleNode->m_gen;

                        auto& info = ruleNode->getInfo();
                        if (!info.empty()) {
                            for (size_t i = 0; i < outputMargin; ++i) {
                                parser->m_ss << " ";
                            }
                            parser->m_ss << "on_rule: " << info.toString() << std::endl;
                        }
                        outputMargin++;
                    }
                }
                
            }
        }
    }

    KAny* MatchR::captureAny(KUSIZE i) {
        if (i >= m_ruleNode->m_gen->m_expStk.size()) {
            return nullptr;
        }
        return &m_ruleNode->m_gen->m_expStk.at(i);
    }

    size_t MatchR::captureSize() {
        return m_ruleNode->m_gen->m_expStk.size();
    };

    std::string MatchR::prefix() {
        const char* globalText = m_ruleNode->m_gen->m_cache;
        return std::string(globalText, globalText + m_startPos);
    }

    std::string MatchR::suffix() {
        const char* globalText = m_ruleNode->m_gen->m_cache;
        return std::string(globalText + m_startPos+ m_length, globalText+ m_ruleNode->m_gen->length);
    }

    std::string MatchR::str() {
        const char* globalText = m_ruleNode->m_gen->m_cache;
        auto start = globalText + m_startPos;
        auto stop = start + m_length;
        if (start == stop) {
            return "";
        }
        if (m_ruleNode->m_gen->m_skipBlank) {
            auto* node = dynamic_cast<RuleCompound*>(this->m_ruleNode);
            if (node) {
                return trim(std::string(start, stop));
            }
        }
        return std::string(start, stop);
    }

    void MatchR::release() {
        visit([](KLib42::Match& m, bool isSink) {
            if (!isSink) {
                delete& m;
            }
            });
    }

    void MatchR::visit(std::function<void(Match& m, bool isSink)> visitor) {
        std::vector<MatchR*> matchers;
        MatchR* curM = this;
        for (; true;) {
            MatchR* m = curM->visitStep();
            if (m) {
                matchers.push_back(curM);
                curM = m;
                try {
                    visitor(*curM, true);
                }
                catch (const std::exception& ex) {
                    std::cerr << ex.what() << "\n";
                }
            }
            else {
                if (matchers.size() == 0) {
                    break;
                }
                auto last = matchers.back();
                matchers.pop_back();
                try {
                    visitor(*curM, false);
                }
                catch (const std::exception& ex) {
                    std::cerr << ex.what() << "\n";
                }
                curM = last;
            }
        }
    };

    MatchR* MatchR::visitStep() {
        return nullptr;
    }

    struct MatchREmpty : public MatchR {
        MatchREmpty(KUSIZE start, RuleNode* rule) 
            :MatchR(start, rule) {
        }

        StepInT stepIn() override {
            if (m_length == LEN::SUCC) {
                return StepInT(false);
            }
            m_length = LEN::SUCC;
            return StepInT(true);
        };
    };

    MatchR* RuleEmpty::match(KUSIZE start) {
        return new MatchREmpty(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //STR    
    struct MatchRStr : public MatchR {
        MatchRStr(KUSIZE start, RuleNode* rule) 
            :MatchR(start, rule) {}
        
        StepInT stepIn() override {
            if (m_length == LEN::FAIL || m_length >= LEN::SUCC) {
                return StepInT(false);
            }
            auto* parser = m_ruleNode->m_gen;
            auto* buff = parser->m_cache;
            auto len = parser->length;
            const char* bptr = buff + m_startPos;
            const char* eptr = buff + len;
            const char* cptr = bptr;
            const RuleStr* rule = static_cast<const RuleStr*>(this->m_ruleNode);
            const char* toMatch = rule->buff;
            auto toMatchEnd = toMatch + rule->len;
            if (toMatch == nullptr) {
                m_length = rule->len;
                return StepInT(true);
            }
            while (true) {
                if (toMatch == toMatchEnd) {
                    m_length = rule->len;
                    return StepInT(true);
                }
                if (cptr == eptr) {
                    break;
                }
                if (*(cptr++) != *(toMatch++)) {
                    m_length = LEN::FAIL;
                    return StepInT(false);
                }
            }
            m_length = LEN::FAIL;
            return StepInT(false);
        };
    };

    MatchR* RuleStr::match(KUSIZE start) {
        return new MatchRStr(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    // pred

    struct MatchRPred : public MatchR {
        MatchRPred(KUSIZE start, RuleNode* rule) 
            :MatchR(start, rule) {
        }
        
        StepInT stepIn() override {
            if (m_length != LEN::INIT) {
                return StepInT{false};
            }
            auto parser = m_ruleNode->m_gen;
            auto pBegin = parser->m_cache;
            auto len = parser->length;
            auto predNode = (RuleCustom*)m_ruleNode;
            auto pred = predNode->pred;
            auto start = pBegin + m_startPos;
            auto last = pBegin + len;
            const char* end = pred(start, last);
            if (end != nullptr) {
                m_length = end - start;
                return StepInT{true};
            }
            else {
                m_length = LEN::FAIL;
                return StepInT(false);
            }
        }
    };

    MatchR* RuleCustom::match(KUSIZE start) {
        return new MatchRPred(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //ANY
    struct MatchRAny : public MatchR {
        int m_curIdx;
        MatchR* m_curMatcher = nullptr;

        MatchRAny(KUSIZE start, RuleNode* rule) 
            :MatchR(start, rule), m_curIdx(-1) {

        }

        void release() override {
            MatchR::release();
            m_curMatcher = nullptr;
        }

        inline bool nextNode() {
            if (m_curMatcher) {
                m_curMatcher->release();
                delete m_curMatcher;
                m_curMatcher = nullptr;
            }
            const RuleAny* thisNode = static_cast<const RuleAny*>(m_ruleNode);
            if(this->m_curIdx+1 == thisNode->children.size()){
                return false;
            }
            auto node = thisNode->children[++this->m_curIdx];
            m_curMatcher = node->match(this->m_startPos);
            return true;
        }
        
        bool visited = false;
        MatchR* visitStep() override{
            if (visited) {
                visited = false;
                return nullptr;
            }
            
            visited = true;
            return m_curMatcher;
        }

        
        StepInT stepIn() override {
            if (m_length == LEN::INIT) {
                if(nextNode()){
                    return StepInT(m_curMatcher);
                } else {
                    m_length = LEN::FAIL;
                    return StepInT(false);
                }
            } else if(m_length == LEN::CERT) {
                m_length = m_curMatcher->m_length;
                return StepInT(true);
            } else if( m_length >= LEN::SUCC ) {
                return m_curMatcher;
            }  
            else {
                m_length = LEN::FAIL;
                return StepInT(false);
            }
        };

        void stepOut(MatchR* r) override {
            if (r) {
                m_length = LEN::CERT;
            }
            else {
                m_length = LEN::INIT;
            }
        };
    };

    MatchR* RuleAny::match(KUSIZE start) {
        return new MatchRAny(start, this);
    }


    //////////////////////////////////////////////////////////////////////////
    // cut
    struct MatchRCut : public MatchR {
        MatchRCut(KUSIZE start, RuleNode* rule)
            :MatchR(start, rule) {
        }

        MatchR* visitStep() override {
            return nullptr;
        }

        StepInT stepIn() override {
            if (m_length == LEN::SUCC) {
                return StepInT(false);
            }
            m_length = LEN::SUCC;
            return StepInT(true);
        };
    };

    MatchR* RuleCut::match(KUSIZE start) {
        return new MatchRCut(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //ALL

    struct MatchRAll : public MatchR {
        MatchRAll(KUSIZE start, RuleNode* rule) 
            :MatchR(start, rule) {
        }
        std::vector<MatchR*> childMatch;
        
        void release() override {
             MatchR::release();
             childMatch.clear();
        }

        bool nextNode() {
            int start = 0;
            if(childMatch.size() == 0){
                start = m_startPos;
            } else {
                auto m = childMatch.back();
                start = m->m_startPos+m->m_length;
            }
            auto* parser = this->m_ruleNode->m_gen;
            auto pBegin = parser->m_cache;
            auto textLen = parser->length;
            auto pEnd = pBegin + textLen;
            const char* cptr = pBegin + start;
            while (true) {
                bool change = false;
                while (cptr != pEnd) {
                    if (!k_isspace(*cptr)) {
                        break;
                    }
                    cptr++;
                    change = true;
                }
                auto skipRule = parser->m_skipRule;
                if (skipRule) {
                    auto* p = skipRule(cptr, pEnd);
                    if (p) {
                        cptr = p;
                        change = true;
                    }
                }
                if (change) {
                    start = cptr - pBegin;
                }
                else {
                    break;
                }
            }
            auto len = childMatch.size();
            const RuleAll* thisNode = static_cast<const RuleAll*>(m_ruleNode);
            if (len == thisNode->children.size()) {
                return false;
            }
            auto node = thisNode->children[len];
            auto matcher = node->match(start);
            childMatch.push_back(matcher);
            return true;
        }

        bool preNode() {
            auto len = childMatch.size();
            if (len == 0) {
                return false;
            }
            auto m = childMatch.back();
            struct MatchRCut* curNode = dynamic_cast<struct MatchRCut*>(m);
            if (curNode) {
                this->release();
                return false;
            }
            m->release();
            delete m;
            childMatch.pop_back();
            if(len == 1) {
                return false;
            }
            return true;
        }

        StepInT stepIn() override {
            if(m_length == LEN::INIT ) {
                    if(nextNode() ) {
                        return StepInT(childMatch.back());
                    } else {
                        auto& m = childMatch.back();
                        m_length = (m->m_startPos+m->m_length)-m_startPos;
                        return StepInT(true);
                    }
                } else if(m_length == LEN::FAIL) {
                    return StepInT(false);
                } else if(m_length == LEN::CERT ) {
                    if(preNode()){
                        m_length = LEN::INIT;
                        return StepInT(childMatch.back());
                    } else {
                        m_length = LEN::FAIL;
                        return StepInT(false);
                    }
                } else if(m_length >= LEN::SUCC)  {
                   return StepInT(childMatch.back());
                }
        };

        void stepOut(MatchR* r) override {
            if (r) {
                m_length = LEN::INIT;
            }
            else {
                m_length = LEN::CERT;
            }
        };

        int visitStepCount = 0;
        MatchR* visitStep() override {
            if (visitStepCount == childMatch.size()) {
                visitStepCount = 0;
                return nullptr;
            }
            else {
                return childMatch[visitStepCount++];
            }
        }
    };

    MatchR* RuleAll::match(KUSIZE start) {
        return new MatchRAll(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //Until

    struct MatchRUntill : public MatchR {
        MatchR* m_curMatcher = nullptr;
        KUSIZE m_maxLen = 0;
        bool m_accept = false;
        MatchRUntill(KUSIZE start, RuleNode* rule)
            :MatchR(start, rule) {
            auto* parser = m_ruleNode->m_gen;
            m_maxLen = parser->length - m_startPos;
        }

        void release() override {
            MatchR::release();
            m_curMatcher = nullptr;
        }

        bool visited = false;
        MatchR* visitStep() override {
            if (visited) {
                visited = false;
                return nullptr;
            }
            visited = true;
            return m_curMatcher;
        }

        StepInT stepIn() override {
            if (m_accept) {
                return StepInT{ false };
            }
            else if (m_curMatcher != nullptr) {
                m_accept = true;
                // clear
                m_curMatcher->release();
                delete m_curMatcher;
                m_curMatcher = nullptr;
                //m_length += m_curMatcher->length();
                return StepInT(true);
            }
            m_length++;
            if (m_length == m_maxLen) {
                m_accept = true;
                m_length = LEN::FAIL;
                return StepInT(false);
            }
            auto* cond = ((RuleUntil*)m_ruleNode)->m_cond;
            m_curMatcher = cond->match(m_startPos + m_length);
            return StepInT(m_curMatcher);
        };

        void stepOut(MatchR* r) override {
            if (r != nullptr) {
                return;
            }
            m_curMatcher->release();
            delete m_curMatcher;
            m_curMatcher = nullptr;
        };
    };

    MatchR* RuleUntil::match(KUSIZE start) {
        return new MatchRUntill(start, this);
    }
    
    //////////////////////////////////////////////////////////////////////////
    //till
    struct MatchRTill : public MatchR {
        MatchR* m_curMatcher = nullptr;
        KUSIZE m_maxLen = 0;
        bool m_accept = false;
        MatchRTill(KUSIZE start, RuleNode* rule)
            :MatchR(start, rule) {
            auto* parser = m_ruleNode->m_gen;
            m_maxLen = parser->length - m_startPos;
        }
        
         void release() override {
             MatchR::release();
             m_curMatcher = nullptr;
         }

        bool visited = false;
        MatchR* visitStep() override {
            if (visited) {
                visited = false;
                return nullptr;
            }
            visited = true;
            return m_curMatcher;
        }

        StepInT stepIn() override {
            if (m_accept) {
                return StepInT{ false };
            }
            else if (m_curMatcher != nullptr) {
                m_accept = true;
                // clear
                /*m_curMatcher->release();
                delete m_curMatcher;
                m_curMatcher = nullptr;*/
                m_length += m_curMatcher->length();
                return StepInT(true);
            }
            m_length++;
            if (m_length == m_maxLen) {
                m_accept = true;
                m_length = LEN::FAIL;
                return StepInT(false);
            }
            auto* cond = ((RuleTill*)m_ruleNode)->m_cond;
            m_curMatcher = cond->match(m_startPos + m_length);
            return StepInT(m_curMatcher);
        };

        void stepOut(MatchR* r) override {
            if (r != nullptr) {
                return;
            }
            m_curMatcher->release();
            delete m_curMatcher;
            m_curMatcher = nullptr;
        };
    };

    MatchR* RuleTill::match(KUSIZE start) {
        return new MatchRTill(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    // not
     struct MatchRNot : public MatchR {
        MatchR* m_curMatcher = nullptr;
        bool m_accept = false;
        MatchRNot(KUSIZE start, RuleNode* rule)
            :MatchR(start, rule) {
        }

        void release() override {
            MatchR::release();
            m_curMatcher = nullptr;
        }
        
        bool visited = false;
        MatchR* visitStep() override {
            if (visited) {
                visited = false;
                return nullptr;
            }
            visited = true;
            return m_curMatcher;
        }
        StepInT stepIn() override {
            if (m_accept) {
                return StepInT{ false };
            }
            else if (m_curMatcher != nullptr) {
                m_accept = true;
                // clear
                m_curMatcher->release();
                delete m_curMatcher;
                m_curMatcher = nullptr;
                m_length = LEN::FAIL;
                return StepInT(false);
            }
            if (m_length == LEN::INIT) {
                m_length = LEN::SUCC;
                auto* cond = ((RuleTill*)m_ruleNode)->m_cond;
                m_curMatcher = cond->match(m_startPos + m_length);
                return StepInT(m_curMatcher);
            }
            else { //m_length = LEN::SUCC;
                m_accept = true;
                return StepInT(true);
            }
        };

        void stepOut(MatchR* r) override {
            if (r != nullptr) {
                return;
            }
            m_curMatcher->release();
            delete m_curMatcher;
            m_curMatcher = nullptr;
        };
    };

    MatchR* RuleNot::match(KUSIZE start) {
        return new MatchRNot(start, this);
    }   

    //////////////////////////////////////////////////////////////////////////
    // char
    struct MatchROne : public MatchR {
        MatchROne(KUSIZE start, RuleNode* rule)
            :MatchR(start, rule) {
        }

        MatchR* visitStep() override {
            return nullptr;
        }

        StepInT stepIn() override {
            if (m_length == 1) {
                return StepInT(false);
            }
            m_length = 1;
            if (m_startPos == m_ruleNode->m_gen->length) {
                return StepInT(false);
            }
            return StepInT(true);
        };
    };

    MatchR* RuleOne::match(KUSIZE start) {
        return new MatchROne(start, this);
    }
}