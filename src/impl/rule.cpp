#include "impl.h"
#include <iostream>
#include <sstream>

namespace KParser {
    MatchR::MatchR(const char* text, size_t length, size_t start, RuleNode* rule) 
        :m_text(text), m_textLen(length), m_startPos(start), m_ruleNode(rule), m_matchLength(LEN::INIT) {
    }

    using LINE = std::tuple<RuleNode*, int, int>; // (node, iden, id)
    using LINES = VecT<LINE>;
    static int idCount = 0;
    static void collectRuleInfo(RuleNode* n, LINES& r, int iden) {
        LINE line;
        std::get<0>(line) = n;
        std::get<1>(line) = iden;
        std::get<2>(line) = -1;
        auto it = std::find_if(r.begin(), r.end(), [&](auto& l) {
            auto lineNode = std::get<0>(l);
            return n == lineNode;
            });
        if (it != r.end()) {
            auto& id = std::get<2>(*it);
            if (id == -1) {
                id = idCount++;
            }
            std::get<2>(line) = id;
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
                return "Str(" + n1->m_text + ")";
            }
        }
        {
            auto n1 = n->as<RuleEmpty>();
            if (n1) {
                return "Empty";
            }
        }
        {
            auto n1 = n->as<RulePred>();
            if (n1) {
                return "Pred";
            }
            return "Unknown";
        }
    }

    RuleNode::RuleNode(ParserImpl* gen) :m_gen(gen) {
        gen->rules.push_back(this);
    }

    RuleNode* RuleNode::on(std::function<void(Match*)> act) {
        m_eval = act;
        return this;
    };

    void RuleNode::appendChild(Rule* r) {
        throw std::exception("unimplemented");
    };

    std::string RuleNode::toString() {
        LINES lns;
        idCount = 0;
        collectRuleInfo(this, lns, 0);
        std::stringstream ss;
        for (auto& ln : lns) {
            auto [node, iden, id] = ln;
            for (auto i = 0; i < iden; ++i) {
                ss << "  ";
            }
            if (id == -1) {
                ss << printRuleLine(node) << "\n";
            }
            else {
                ss << printRuleLine(node) << ":" << id << "\n";
            }
        }
        return ss.str();
    }

    std::unique_ptr<Match> RuleNode::parse(const std::string& text) {
        m_gen->reset();
        this->m_gen->m_toparse = text;
        auto m = this->match(this->m_gen->m_toparse.c_str(), this->m_gen->m_toparse.length(), 0);
        std::unique_ptr<Match> um;
        um.reset(m);
        auto r = m->alter();
        if (!r) {
            return nullptr;
        }
        
        um->visit([](auto* m) {
            auto mr = (MatchR*)m;
            auto f = mr->m_ruleNode->m_eval;
            if (f) {
                try {
                    f(mr);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
            }
            delete mr;
        });
        auto f = m->m_ruleNode->m_eval;
        if (f) {
            try {
                f(m);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
        return um;
    }

    //////////////////////////////////////////////////////////////////////////
    //EMPTY
    class RuleEmptyCLS : public CLSINFO {
        StrT getName() override {
            return "Empty";
        }
    };

    CLSINFO* RuleEmpty::CLS() {
        // note, not delete!
        static RuleEmptyCLS cls;
        return &cls;
    }

    bool MatchR::alter() {
        VecT<MatchR*> vec;
        MatchR* curM = this;
        auto lookback = m_ruleNode->m_gen->m_lookback;
        auto& headMax = m_ruleNode->m_gen->m_headMax;
        
        while (true) {
            auto st = curM->stepIn();
            if (st.index() == 0) {
                auto stBool = std::get<bool>(st);
                if (curM == this) {
                    return stBool;
                }
                else {
                    MatchR* lastM = curM;
                    curM = vec.back();
                    vec.pop_back();
                    if (stBool) {
                        auto headLength = lastM->m_startPos + lastM->size();
                        if (headLength > headMax) {
                            headMax = headLength;
                        }
                        curM->stepOut(lastM);
                    }
                    else {
                        const CLSINFO* lastClass = lastM->m_ruleNode->getCLS();
                        if ((int)headMax - (int)lastM->m_startPos> (int)lookback) {
                            if (lastClass != RuleAll::CLS()
                                || lastClass != RuleAny::CLS()) {
                                // clear all temporary matcher
                                for (int i = vec.size() - 1; i > -1; --i) {
                                    vec[i]->release();
                                }
                                std::cerr << "lookback max than " << headMax << "chars, abandon\n";
                                return false;
                            }
                        }
                        curM->stepOut(nullptr);
                    }
                }
            }
            else {
                MatchR* matcher = std::get<MatchR*>(st);
                vec.push_back(curM);
                curM = matcher;
            }
        }
    }

    StrT MatchR::str() {
        auto start = m_text + m_startPos;
        auto end = start + size();
        if (m_ruleNode->m_gen->m_skipBlank) {
            while (true) {
                if (start == end) {
                    return "";
                }
                if (!std::isblank(*start)) {
                    break;
                }
                ++start;
            }
        }
        return StrT(start, end);
    }

    std::any& MatchR::value() {
        return m_val;
    }

    void MatchR::release() {
        visit([](auto* m) {
            delete m;
            });
    }

    void MatchR::visit(std::function<void(Match* m)> visitor) {
        std::vector<MatchR*> matchers;
        MatchR* curM = this;
        for (; true;) {
            auto m = curM->visitStep();
            if (m) {
                matchers.push_back(curM);
                curM = m;
            }
            else {
                if (matchers.size() == 0) {
                    break;
                }
                auto last = matchers.back();
                matchers.pop_back();
                try {
                    visitor(curM);
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
        MatchREmpty(const char* text, size_t length, size_t start, RuleNode* rule) 
            :MatchR(text, length, start, rule) {
        }
        std::variant<bool, MatchR*> stepIn() override {
            if (m_matchLength == LEN::SUCC) {
                return false;
            }
            m_matchLength = LEN::SUCC;
            return true;
        };
    };

    MatchR* RuleEmpty::match(const char* text, size_t length, size_t start) {
        return new MatchREmpty(text, length, start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //STR
    class RuleStrCLS : public CLSINFO {
        StrT getName() override {
            return "Str";
        }
    };

    struct MatchRStr : public MatchR {
        MatchRStr(const char* text, size_t length, size_t start, RuleNode* rule) 
            :MatchR(text, length, start, rule) {
        }
        
        std::any& value() override {
            m_val = ((RuleStr*)m_ruleNode)->m_text;
            return m_val;
        }

        std::variant<bool, MatchR*> stepIn() override {
            if (m_matchLength == LEN::FAIL || m_matchLength >= LEN::SUCC) {
                return false;
            }
            const char* cptr = m_text + m_startPos;
            const RuleStr* rule = static_cast<const RuleStr*>(this->m_ruleNode);
            const std::string& tomatch = rule->m_text;
            auto tomatchLen = tomatch.length();
            auto text_len = m_textLen;
            auto i = 0;
            while (true) {
                if (i == tomatchLen) {
                    break;
                }
                if (*(cptr++) != tomatch[i++]) {
                    m_matchLength = LEN::FAIL;
                    return false;
                }
                else if (m_startPos + i > text_len) {
                    m_matchLength = LEN::FAIL;
                    return false;
                }
            }
            m_matchLength = tomatchLen;
            return true;
        };
    };

    MatchR* RuleStr::match(const char* text, size_t length, size_t start) {
        return new MatchRStr(text, length, start, this);
    }

    CLSINFO* RuleStr::CLS() {
        static RuleStrCLS cls;
        return &cls;
    }

    //////////////////////////////////////////////////////////////////////////
    // pred

    class RulePredCLS : public CLSINFO {
        StrT getName() override {
            return "Pred";
        }
    };

    CLSINFO* RulePred::CLS() {
        static RulePredCLS cls;
        return &cls;
    }

    struct MatchRPred : public MatchR {
        MatchRPred(const char* text, size_t length, size_t start, RuleNode* rule) 
            :MatchR(text, length, start, rule) {
        }
        std::any m_val;
        std::any& value() override {
            return m_val;
        }
        std::variant<bool, MatchR*> stepIn() override {
            if (m_matchLength != LEN::INIT) {
                return false;
            }
            auto predNode = (RulePred*)m_ruleNode;
            auto pred = predNode->pred;
            auto start = m_text + m_startPos;
            auto last = m_text + m_textLen;
            const char* end = pred(m_text + m_startPos, m_text + m_textLen, m_val);
            if (end != nullptr) {
                m_matchLength = end - start;
                return true;
            }
            else {
                m_matchLength = LEN::FAIL;
                return false;
            }
        }
    };

    MatchR* RulePred::match(const char* text, size_t length, size_t start) {
        return new MatchRPred(text, length, start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //ANY

    class RuleAnyCLS : public CLSINFO {
        StrT getName() override {
            return "Any";
        }
    };

    CLSINFO* RuleAny::CLS() {
        static RuleAnyCLS cls;
        return &cls;
    }


    struct MatchRAny : public MatchR {
        int m_curIdx;
        MatchR* m_curMatcher = nullptr;

        MatchRAny(const char* text, size_t length, size_t start, RuleNode* rule) 
            :MatchR(text, length, start, rule), m_curIdx(-1) {
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
            m_curMatcher = node->match(this->m_text, this->m_textLen, this->m_startPos);
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
        std::variant<bool, MatchR*> stepIn() override {
            if (m_matchLength >= LEN::SUCC && fromStepOut) {
                fromStepOut = false;
                return true;
            }
            if (!m_curMatcher) {
                m_matchLength = LEN::FAIL;
                return false;
            }
            
            return m_curMatcher;
        };

        void stepOut(MatchR* r) override {
            if (r != nullptr) {
                m_matchLength = m_curMatcher->size();
                fromStepOut = true;
            }
            else {
                if (!nextNode()) {
                    m_matchLength = LEN::FAIL;
                    m_curMatcher->release();
                    delete m_curMatcher;
                    m_curMatcher = nullptr;
                }
            }
        };
    };

    MatchR* RuleAny::match(const char* text, size_t length, size_t start) {
        return new MatchRAny(text, length, start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //ALL
    class RuleAllCLS : public CLSINFO {
        StrT getName() override {
            return "All";
        }
    };

    const CLSINFO* RuleAll::CLS() {
        static RuleAllCLS cls;
        return &cls;
    };

    struct MatchRAll : public MatchR {
        MatchRAll(const char* text, size_t length, size_t start, RuleNode* rule) 
            :MatchR(text, length, start, rule) {
            m_curStart = start;
            nextNode();
        }
        std::vector<MatchR*> childMatch;
        size_t m_curStart;
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
                auto textLen = this->m_textLen;
                const char* cptr = m_text + m_curStart;
                while (true) {
                    if (m_curStart > textLen) {
                        break;
                    }
                    if (m_curStart == textLen) {
                        break;
                    }
                    if (!std::isblank(*cptr)) {
                        break;
                    }
                    cptr++;
                    m_curStart = cptr - m_text;
                }
            }
            auto len = childMatch.size();
            const RuleAll* thisNode = static_cast<const RuleAll*>(m_ruleNode);
            if (len == thisNode->children.size()) {
                return false;
            }
            auto node = thisNode->children[len];
            auto matcher = node->match(this->m_text, this->m_textLen, this->m_curStart);

            childMatch.push_back(matcher);
            return true;
        }

        bool preNode() {
            auto len = childMatch.size();
            if (len == 0) {
                return false;
            }
            auto m = childMatch.back();
            m->visit([](auto* child) {
                delete child;
                });
            delete m;
            childMatch.pop_back();
            return true;
        }

        bool fromStepOut = false;
        std::variant<bool, MatchR*> stepIn() override {
            if ( m_matchLength >= LEN::SUCC && fromStepOut) {
                fromStepOut = false;
                return true;
            }
            else if (m_matchLength == LEN::FAIL) {
                return false;
            }
            auto len = childMatch.size();
            if (len == 0) {
                m_matchLength = LEN::FAIL;
                return false;
            }

            MatchR* matcher = childMatch.back();
            m_curStart = matcher->m_startPos;
            return matcher;
        };

        void stepOut(MatchR* r) override {
            if (r != nullptr) {
                MatchR* matcher = childMatch.back();
                m_curStart = m_curStart + matcher->size();
                if (!nextNode()) {
                    m_matchLength = m_curStart - m_startPos;
                    fromStepOut = true;
                }
            }
            else {
                if (!preNode()) {
                    m_matchLength = LEN::FAIL;
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

    MatchR* RuleAll::match(const char* text, size_t length, size_t start) {
        return new MatchRAll(text, length, start, this);
    }

}