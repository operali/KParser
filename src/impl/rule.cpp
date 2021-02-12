
#include <algorithm>
#include <iostream>
#include <sstream>
#include "util.h"
#include "impl.h"

// #define isspace(ch) ((ch) == ' ' || (ch) == '\t' || (ch) == '\r' || (ch) == '\n' || (ch) == '\v')


namespace KParser {
    struct LINE {
        RuleNode* node;
        int32_t iden;
        int32_t id;
    };

    MatchR::MatchR(uint32_t start, RuleNode* rule) 
        :m_startPos(start), m_ruleNode(rule), m_length(LEN::INIT) {
    }

    using LINES = VecT<LINE>;
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
        auto allN = n->as<RuleAll>();
        if (allN) {
            for (auto c : allN->children) {
                collectRuleInfo(c, r, iden + 1);
            }
        }
        else {
            auto anyN = n->as<RuleAny>();
            if (anyN) {
                for (auto c : anyN->children) {
                    collectRuleInfo(c, r, iden + 1);
                }
            }
        }
    }

    static std::string printRuleLine(RuleNode* n) {
        auto n1 = n->as<RuleAll>();
        if (n1) {
            return "All";
        }
        {
            auto n1 = n->as<RuleAny>();
            if (n1) {
                return "Any";
            }
        }
        {
            auto n1 = n->as<RuleStr>();
            if (n1) {
                return "Str(" +std::string(n1->buff, n1->buff + n1->len) + ")";
            }
        }
        {
            auto n1 = n->as<RuleEmpty>();
            if (n1) {
                return "Empty";
            }
        }
        {
            auto n1 = n->as<RuleCustom>();
            if (n1) {
                return "Custom";
            }
            return "Unknown";
        }
    }

    RuleNode::RuleNode(ParserImpl* gen) :m_gen(gen) {
        gen->rules.push_back(this);
    }

    RuleNode* RuleNode::visit(std::function<void(Match&, bool)> act) {
        m_visitHandle = act;
        return this;
    };

    RuleNode* RuleNode::eval(std::function<libany::any(Match& m, IT arg, IT noarg)> eval){
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

    std::unique_ptr<Match> RuleNode::parse(const std::string& text) {
        m_gen->reset();
        this->m_gen->setText(text);

        auto m = this->match(0);
        std::unique_ptr<Match> um;
        um.reset(m);

        std::vector<libany::any>& expStk = m_gen->m_expStk;
        using IT = std::vector<libany::any>::iterator;
        std::vector<uint32_t> opStk;

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

        m->visit([&](KParser::Match& m, bool is_begin) {
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
                auto eval = ((MatchR*)&m)->m_ruleNode->m_evalHandle;
                if (eval) {
                    libany::any res;
                    auto from = opStk.back();
                    IT b = expStk.begin() + from;
                    try {
                        auto v = eval(*mr, b, expStk.end());
                        std::swap(v, res);
                    }
                    catch (std::exception& e) {
                        std::cerr << e.what() << std::endl;
                    }
                    expStk.erase(b, expStk.end());
                    expStk.emplace_back(std::move(res));
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
            libany::any res;
            auto from = opStk.back();
            IT b = expStk.begin() + from;
            try {
                auto v = eval(*m, b, expStk.end());
                std::swap(v, res);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            expStk.erase(b, expStk.end());
            expStk.emplace_back(std::move(res));
            opStk.pop_back();
        }
        return um;
    }

    //////////////////////////////////////////////////////////////////////////
    //EMPTY

    bool MatchR::alter() {
        VecT<MatchR*> vec;
        MatchR* curM = this;
        auto* gen = m_ruleNode->m_gen;
        auto lookback = gen->m_lookback;
        auto& headMax = gen->m_headMax;
        auto* text = gen->m_cache;
                
        while (true) {
            auto st = curM->stepIn();
            if (st.mr == nullptr) {
                auto succ = st.res;
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
                        auto headLength = lastM->m_startPos + lastM->length();
                        if (headLength > headMax) {
                            headMax = headLength;
                        }
                        curM->stepOut(lastM);
                    }
                    else {
                        if ((int)headMax - (int)lastM->m_startPos> (int)lookback) {
                            auto* node = dynamic_cast<RuleCompound*>(lastM->m_ruleNode);
                            if (node) {
                                // clear all temporary matcher
                                for (int i = vec.size() - 1; i > -1; --i) {
                                    vec[i]->release();
                                }
                                std::cerr << "lookback more than max length of " << headMax << " chars, abandon\n";
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
                vec.push_back(curM);
                curM = matcher;
            }
        }
    }

    libany::any* MatchR::capture(uint32_t i) {
        if (i >= m_ruleNode->m_gen->m_expStk.size()) {
            return nullptr;
        }
        return &m_ruleNode->m_gen->m_expStk.at(i);
    }

    std::string MatchR::errInfo() {
        return m_ruleNode->m_gen->m_interface->errInfo();
    }

    StrT MatchR::prefix() {
        const char* globalText = m_ruleNode->m_gen->m_cache;
        return StrT(globalText, globalText + m_startPos);
    }

    StrT MatchR::suffix() {
        const char* globalText = m_ruleNode->m_gen->m_cache;
        return StrT(globalText + m_startPos+ m_length, globalText+ m_ruleNode->m_gen->length);
    }

    StrT MatchR::occupied_str() {
        const char* globalText = m_ruleNode->m_gen->m_cache;
        auto start = globalText + m_startPos;
        auto stop = start + length();

        return StrT(start, stop);
    }

    StrT MatchR::str() {
        const char* globalText = m_ruleNode->m_gen->m_cache;
        auto start = globalText + m_startPos;
        auto stop = start + m_length;
        if (start == stop) {
            return "";
        }
        if (m_ruleNode->m_gen->m_skipBlank) {
            auto* node = dynamic_cast<RuleCompound*>(this->m_ruleNode);
            if (node) {
                return trim(StrT(start, stop));
            }
        }
        return StrT(start, stop);
    }

    void MatchR::release() {
        visit([](KParser::Match& m, bool capture) {
            if (!capture) {
                delete& m;
            }
            });
    }

    void MatchR::visit(std::function<void(Match& m, bool capture)> visitor) {
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
        MatchREmpty(uint32_t start, RuleNode* rule) 
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

    MatchR* RuleEmpty::match(uint32_t start) {
        return new MatchREmpty(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //STR    
    struct MatchRStr : public MatchR {
        MatchRStr(uint32_t start, RuleNode* rule) 
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

    MatchR* RuleStr::match(uint32_t start) {
        return new MatchRStr(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    // pred

    struct MatchRPred : public MatchR {
        MatchRPred(uint32_t start, RuleNode* rule) 
            :MatchR(start, rule) {
        }
        
        StepInT stepIn() override {
            if (m_length != LEN::INIT) {
                return StepInT{false};
            }
            auto parser = m_ruleNode->m_gen;
            auto ptext = parser->m_cache;
            auto len = parser->length;
            auto predNode = (RuleCustom*)m_ruleNode;
            auto pred = predNode->pred;
            auto start = ptext + m_startPos;
            auto last = ptext + len;
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

    MatchR* RuleCustom::match(uint32_t start) {
        return new MatchRPred(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //ANY
    struct MatchRAny : public MatchR {
        int m_curIdx;
        MatchR* m_curMatcher = nullptr;

        MatchRAny(uint32_t start, RuleNode* rule) 
            :MatchR(start, rule), m_curIdx(-1) {
            nextNode();
        }

        ~MatchRAny() {
            /*if (m_curMatcher) {
                delete m_curMatcher;
                m_curMatcher = nullptr;
            }*/
        }

         void release() override {
             MatchR::release();
             m_curMatcher = nullptr;
         }

        inline bool nextNode() {
            this->m_curIdx++;
            const RuleAny* thisNode = static_cast<const RuleAny*>(m_ruleNode);
            if (this->m_curIdx == thisNode->children.size()) {
                if (!m_curMatcher) {
                    m_curMatcher->release();
                    delete m_curMatcher;
                    m_curMatcher = nullptr;
                }
                return false;
            }
            auto node = thisNode->children[this->m_curIdx];
            if (m_curMatcher != nullptr) {
                delete m_curMatcher;
                m_curMatcher = nullptr;
            }
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

        bool fromStepOut = false;
        StepInT stepIn() override {
            if (m_length >= LEN::SUCC && fromStepOut) {
                fromStepOut = false;
                return StepInT(true);
            }
            if (!m_curMatcher) {
                m_length = LEN::FAIL;
                return StepInT(false);
            }
            return StepInT(m_curMatcher);
        };

        void stepOut(MatchR* r) override {
            if (r != nullptr) {
                m_length = m_curMatcher->length();
                fromStepOut = true;
            }
            else {
                if (!nextNode()) {
                    m_length = LEN::FAIL;
                    m_curMatcher->release();
                    delete m_curMatcher;
                    m_curMatcher = nullptr;
                }
            }
        };
    };

    MatchR* RuleAny::match(uint32_t start) {
        return new MatchRAny(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //ALL

    struct MatchRAll : public MatchR {
        MatchRAll(uint32_t start, RuleNode* rule) 
            :MatchR(start, rule) {
            m_curStart = start;
            nextNode();
        }
        std::vector<MatchR*> childMatch;
        uint32_t m_curStart;
        ~MatchRAll() {
            /*for (auto m : childMatch) {
                delete m;
            }
            childMatch.resize(0);
            childMatch.clear();*/
        }
        
        void release() override {
             MatchR::release();
             childMatch.clear();
        }

        bool nextNode() {
            if (this->m_ruleNode->m_gen->m_skipBlank) {
                auto parser = m_ruleNode->m_gen;
                auto ptext = parser->m_cache;
                auto textLen = parser->length;
                const char* cptr = ptext + m_curStart;
                while (true) {
                    if (m_curStart > textLen) {
                        break;
                    }
                    if (m_curStart == textLen) {
                        break;
                    }
                    if (!isspace(*cptr)) {
                        break;
                    }
                    cptr++;
                    m_curStart = cptr - ptext;
                }
            }
            auto len = childMatch.size();
            const RuleAll* thisNode = static_cast<const RuleAll*>(m_ruleNode);
            if (len == thisNode->children.size()) {
                return false;
            }
            auto node = thisNode->children[len];
            auto matcher = node->match(this->m_curStart);

            childMatch.push_back(matcher);
            return true;
        }

        bool preNode() {
            auto len = childMatch.size();
            if (len == 0) {
                return false;
            }
            auto m = childMatch.back();
            m->visit([](Match& child, bool capture) {
                if (!capture) {
                    delete& child;
                }
                });
            delete m;
            childMatch.pop_back();
            return true;
        }

        bool fromStepOut = false;
        StepInT stepIn() override {
            if ( m_length >= LEN::SUCC && fromStepOut) {
                fromStepOut = false;
                return StepInT{ true};
            }
            else if (m_length == LEN::FAIL) {
                return StepInT{ false };
            }
            auto len = childMatch.size();
            if (len == 0) {
                m_length = LEN::FAIL;
                return StepInT{ false };
            }

            MatchR* matcher = childMatch.back();
            m_curStart = matcher->m_startPos;
            return StepInT{ matcher };
        };

        void stepOut(MatchR* r) override {
            if (r != nullptr) {
                MatchR* matcher = childMatch.back();
                m_curStart = m_curStart + matcher->length();
                if (!nextNode()) {
                    m_length = m_curStart - m_startPos;
                    fromStepOut = true;
                }
            }
            else {
                if (!preNode()) {
                    m_length = LEN::FAIL;
                }
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

    MatchR* RuleAll::match(uint32_t start) {
        return new MatchRAll(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //Until

    struct MatchRUntill : public MatchR {
        MatchR* m_curMatcher = nullptr;
        uint32_t m_maxLen = 0;
        bool m_accept = false;
        MatchRUntill(uint32_t start, RuleNode* rule)
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

    MatchR* RuleUntil::match(uint32_t start) {
        return new MatchRUntill(start, this);
    }
    


    //////////////////////////////////////////////////////////////////////////
    //till

    struct MatchRTill : public MatchR {
        MatchR* m_curMatcher = nullptr;
        uint32_t m_maxLen = 0;
        bool m_accept = false;
        MatchRTill(uint32_t start, RuleNode* rule)
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

    MatchR* RuleTill::match(uint32_t start) {
        return new MatchRTill(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    // not
     struct MatchRNot : public MatchR {
        MatchR* m_curMatcher = nullptr;
        bool m_accept = false;
        MatchRNot(uint32_t start, RuleNode* rule)
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

    MatchR* RuleNot::match(uint32_t start) {
        return new MatchRNot(start, this);
    }   

    //////////////////////////////////////////////////////////////////////////
    // char
    struct MatchROne : public MatchR {
        MatchROne(uint32_t start, RuleNode* rule)
            :MatchR(start, rule) {
        }

        bool visited = false;
        MatchR* visitStep() override {
            if (visited) {
                visited = false;
                return nullptr;
            }
            visited = true;
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

    MatchR* RuleOne::match(uint32_t start) {
        return new MatchROne(start, this);
    }
}