#include "impl.h"
#include <iostream>
#include <sstream>

// #define isspace(ch) ((ch) == ' ' || (ch) == '\t' || (ch) == '\r' || (ch) == '\n' || (ch) == '\v')
#define isspace(ch) (std::isspace(ch) != 0)

std::string trim(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && isspace(*it))
        it++;

    std::string::const_reverse_iterator rit = s.rbegin();
    while (rit.base() != it && isspace(*rit))
        rit++;

    return std::string(it, rit.base());
}

namespace KParser {
    MatchR::MatchR(size_t start, RuleNode* rule) 
        :m_startPos(start), m_matchstartPos(start), m_matchstopPos(start), m_ruleNode(rule), m_length(LEN::INIT) {
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

    RuleNode* RuleNode::visit(std::function<void(Match&, bool)> act) {
        m_visitHandle = act;
        return this;
    };

    RuleNode* RuleNode::eval(std::function<std::any(Match& m, IT arg, IT noarg)> eval){
        m_evalHandle = eval;
        return this;
    }

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
        this->m_gen->setText(text);

        auto m = this->match(0);
        std::unique_ptr<Match> um;
        um.reset(m);

        std::vector<std::any> expStk;
        using IT = std::vector<std::any>::iterator;
        std::vector<size_t> opStk;

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
        
        m->visit([&](auto& m, bool is_begin) {
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
                    std::any res;
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
            try {
                auto v = eval(*m, expStk.begin(), expStk.end());
                std::swap(v, m_gen->m_value);
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
                        auto headLength = lastM->m_startPos + lastM->length();
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
                                std::cerr << "lookback more than max length of " << headMax << " chars, abandon\n";
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
    DataStack& MatchR::global_data() {
        return m_ruleNode->m_gen->dataStk;
    }

    const char* MatchR::global_text() {
        return m_ruleNode->m_gen->m_cache;
    }

    std::any& MatchR::global_value() {
        return m_ruleNode->m_gen->m_value;
    }

    StrT MatchR::prefix() {
        return StrT(global_text(), global_text() + m_startPos);
    }

    StrT MatchR::suffix() {
        return StrT(global_text(), global_text() + m_startPos+ m_length);
    }

    StrT MatchR::occupied_str() {
        auto start = global_text() + m_startPos;
        auto stop = start + length();
        /*const char* left = start;
        const char* right = stop-1;
        if (m_ruleNode->m_gen->m_skipBlank) {
            while (left < stop) {
                if (!isspace(*left)) {
                    break;
                }
                ++left;
            }
            while (left < right) {
                if (!isspace(*right)) {
                    break;
                }
                --right;
            }
        }*/
        //return StrT(left, right+1);
        return StrT(start, stop);
    }

    StrT MatchR::str() {
        auto start = global_text() + m_matchstartPos;
        auto stop = global_text() + m_matchstopPos;
        if (m_matchstartPos == m_matchstopPos) {
            return "";
        }
        if (m_ruleNode->m_gen->m_skipBlank) {
            auto cls = this->m_ruleNode->getCLS();
            
            if (cls == RuleAll::CLS() 
                || cls == RuleAny::CLS()) {
                return trim(StrT(start, stop));
            }
        }
        return StrT(start, stop);
    }

    void MatchR::release() {
        visit([](auto& m, bool capture) {
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
        MatchREmpty(size_t start, RuleNode* rule) 
            :MatchR(start, rule) {
        }
        std::variant<bool, MatchR*> stepIn() override {
            if (m_length == LEN::SUCC) {
                return false;
            }
            m_length = LEN::SUCC;
            return true;
        };
    };

    MatchR* RuleEmpty::match(size_t start) {
        return new MatchREmpty(start, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //STR
    class RuleStrCLS : public CLSINFO {
        StrT getName() override {
            return "Str";
        }
    };

    struct MatchRStr : public MatchR {
        MatchRStr(size_t start, RuleNode* rule) 
            :MatchR(start, rule) {
        }
        
        std::variant<bool, MatchR*> stepIn() override {
            if (m_length == LEN::FAIL || m_length >= LEN::SUCC) {
                return false;
            }
            auto parser = m_ruleNode->m_gen;
            const char* bptr = parser->m_cache + m_startPos;
            const char* cptr = bptr;
            const char* eptr = parser->m_cache + parser->length;
            const RuleStr* rule = static_cast<const RuleStr*>(this->m_ruleNode);
            const char* toMatch = rule->buff;
            auto toMatchEnd = toMatch + rule->len;
            auto text_len = parser->length;
            if (toMatch == nullptr) {
                m_length = rule->len;
                return true;
            }
            while (true) {
                if (toMatch == toMatchEnd) {
                    m_matchstopPos = cptr - parser->m_cache;
                    m_length = rule->len;
                    return true;
                }
                if (cptr == eptr) {
                    break;
                }
                if (*(cptr++) != *(toMatch++)) {
                    m_length = LEN::FAIL;
                    return false;
                }
            }
            m_length = LEN::FAIL;
            return false;
        };
    };

    MatchR* RuleStr::match(size_t start) {
        return new MatchRStr(start, this);
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
        MatchRPred(size_t start, RuleNode* rule) 
            :MatchR(start, rule) {
        }
        
        std::variant<bool, MatchR*> stepIn() override {
            if (m_length != LEN::INIT) {
                return false;
            }
            auto parser = m_ruleNode->m_gen;
            auto ptext = parser->m_cache;
            auto len = parser->length;
            auto predNode = (RulePred*)m_ruleNode;
            auto pred = predNode->pred;
            auto start = ptext + m_startPos;
            auto last = ptext + len;
            const char* end;
            const char* matchStart;
            const char* matchStop;
            pred(start, last, matchStart, matchStop, end);
            if (end != nullptr) {
                m_matchstartPos = matchStart - start + m_startPos;
                m_matchstopPos = matchStop - start + m_startPos;
                m_length = end - start;
                return true;
            }
            else {
                m_length = LEN::FAIL;
                return false;
            }
        }
    };

    MatchR* RulePred::match(size_t start) {
        return new MatchRPred(start, this);
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

        MatchRAny(size_t start, RuleNode* rule) 
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
        std::variant<bool, MatchR*> stepIn() override {
            if (m_length >= LEN::SUCC && fromStepOut) {
                fromStepOut = false;
                m_matchstopPos = m_startPos + m_length;
                return true;
            }
            if (!m_curMatcher) {
                m_length = LEN::FAIL;
                return false;
            }
            
            return m_curMatcher;
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

    MatchR* RuleAny::match(size_t start) {
        return new MatchRAny(start, this);
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
        MatchRAll(size_t start, RuleNode* rule) 
            :MatchR(start, rule) {
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
            m->visit([](auto& child, bool capture) {
                if (!capture) {
                    delete& child;
                }
                });
            delete m;
            childMatch.pop_back();
            return true;
        }

        bool fromStepOut = false;
        std::variant<bool, MatchR*> stepIn() override {
            if ( m_length >= LEN::SUCC && fromStepOut) {
                fromStepOut = false;
                m_matchstopPos = m_startPos + m_length;
                return true;
            }
            else if (m_length == LEN::FAIL) {
                return false;
            }
            auto len = childMatch.size();
            if (len == 0) {
                m_length = LEN::FAIL;
                return false;
            }

            MatchR* matcher = childMatch.back();
            m_curStart = matcher->m_startPos;
            return matcher;
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

    MatchR* RuleAll::match(size_t start) {
        return new MatchRAll(start, this);
    }
}