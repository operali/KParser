#include <string>
#include <sstream>
#include <iostream>
#include <regex>
#include "KParser.h"

namespace KParser {
    size_t Parser::count = 0;
    struct Data {
        VecT<std::any> stk;

        template<typename T>
        inline void push(const T&& d) {
            stk.push_back(d);
        }

        template<typename T>
        inline T get(size_t i) {
            if (i >= stk.size()) {
                throw std::exception();
            }
            return std::any_cast<T>(stk[i]);
        }

        template<typename T>
        inline void set(T&& t) {
            stk.clear();
            stk.push_back(t);
        }
    };

    UPtrT<Parser> create() {
        auto p = std::make_unique<Parser>();
        return p;
    }

    MatchR::MatchR(const char* text, size_t length, size_t start, const RuleNode* rule) :m_text(text), m_textLen(length), m_startPos(start), m_ruleNode(rule), m_matchLength(LEN::INIT) {
        rule->m_gen->matchers.push_back(this);
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
    std::string printRule(RuleNode* n) {
        LINES lns;
        idCount = 0;
        collectRuleInfo(n, lns, 0);
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

    RuleNode::RuleNode(Parser* gen) :m_gen(gen) {
        gen->rules.push_back(this);
    }

    MatchR* RuleNode::parse(const std::string& text) {
        this->m_gen->m_toparse = text;
        auto m = this->match(&(*this->m_gen->m_toparse.begin()), text.length(), 0);
        auto r = m->alter();
        if (!r)return nullptr;
        m->visit();
        return m;
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

    void MatchR::visit() {
        auto f = this->m_ruleNode->m_eval;
        if (f) {
            try {
                f(this);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
    };

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

    struct MatchREmpty : public MatchR {
        MatchREmpty(const char* text, size_t length, size_t start, const RuleNode* rule) :MatchR(text, length, start, rule) {
        }

        bool alter() final {
            if (m_matchLength == LEN::SUCC) {
                return false;
            }
            m_matchLength = LEN::SUCC;
            return true;
        }
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
        MatchRStr(const char* text, size_t length, size_t start, const RuleNode* rule) :MatchR(text, length, start, rule) {
        }
        
        std::any& value() override {
            m_val = ((RuleStr*)m_ruleNode)->m_text;
            return m_val;
        }

        bool alter() final {
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
        }
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
        MatchRPred(const char* text, size_t length, size_t start, const RuleNode* rule) :MatchR(text, length, start, rule) {
        }
        std::any m_val;
        std::any& value() override {
            return m_val;
        }
        void visit() override {
            auto f = m_ruleNode->m_eval;
            if (f) {
                try {
                    f(this);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
            }
        };
        bool alter() override {
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
        MatchR* m_curMatcher;

        MatchRAny(const char* text, size_t length, size_t start, const RuleNode* rule) :MatchR(text, length, start, rule), m_curIdx(-1) {
            const RuleAny* thisNode = static_cast<const RuleAny*>(m_ruleNode);
            nextNode();
        }

        bool nextNode() {
            this->m_curIdx++;
            const RuleAny* thisNode = static_cast<const RuleAny*>(m_ruleNode);
            if (this->m_curIdx == thisNode->children.size()) {
                m_curMatcher = nullptr;
                return false;
            }
            auto node = thisNode->children[this->m_curIdx];
            m_curMatcher = node->match(this->m_text, this->m_textLen, this->m_startPos);
            return true;
        }
        void visit() override {
            auto f = m_ruleNode->m_eval;
            this->m_curMatcher->visit();

            if (f) {
                try {
                    f(this);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
            }


        };
        bool alter() override {
            if (!m_curMatcher) {
                m_matchLength = LEN::FAIL;
                return false;
            }
            auto r = m_curMatcher->alter();

            if (r) {
                m_matchLength = m_curMatcher->size();
                return r;
            }
            if (!nextNode()) {
                m_matchLength = LEN::FAIL;
                return false;
            }

            return alter();
        }
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

    CLSINFO* RuleAll::CLS() {
        static RuleAllCLS cls;
        return &cls;
    };

    struct MatchRAll : public MatchR {
        MatchRAll(const char* text, size_t length, size_t start, const RuleNode* rule) :MatchR(text, length, start, rule) {
            m_curStart = start;
            nextNode();
        }
        std::vector<MatchR*> childMatch;
        size_t m_curStart;

        bool nextNode() {
            if (this->m_ruleNode->m_gen->m_skipBlank) {
                auto textLen = this->m_textLen;
                const char* cptr = m_text + m_curStart;
                while (true) {
                    if (m_curStart == textLen) {
                        break;
                    }
                    if (!std::isblank(*cptr)) {
                        break;
                    }
                    cptr++;
                }
                m_curStart = cptr - m_text;
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
            childMatch.pop_back();
            return true;
        }
        void visit() override {
            auto f = m_ruleNode->m_eval;
            for (auto& m : this->childMatch) {
                m->visit();
            }
            if (f) {
                try {
                    f(this);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
            }

        };
        bool alter() override {
            auto len = childMatch.size();
            if (len == 0) {
                return false;
            }
            auto matcher = childMatch.back();

            auto r = matcher->alter();
            if (r) {
                m_curStart = m_curStart + matcher->size();
                if (!nextNode()) {
                    m_matchLength = m_curStart - m_startPos;
                    return true;
                }
            }
            else if (!preNode()) {
                m_matchLength = LEN::FAIL;
                return false;
            }
            return alter();
        }
    };

    MatchR* RuleAll::match(const char* text, size_t length, size_t start) {
        return new MatchRAll(text, length, start, this);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // regex
    // n* = (n + n*) | epsilon;
    Parser::~Parser() {
        for (auto m : matchers) {
            delete m;
        }
        matchers.resize(0);
        for (auto r : rules) {
            delete r;
        }
        rules.resize(0);
        Parser::count--;
    }

    RuleNode* Parser::many(RuleNode* node) {
        auto kstart = this->any();
        auto epsilon = this->none();
        auto k_kstart = this->all();
        k_kstart->cons(node, kstart);
        kstart->cons(k_kstart, epsilon);
        return kstart;
    }

    // n+ = n + n*
    RuleNode* Parser::many1(RuleNode* node) {
        auto nstart = this->many(node);
        auto nplus = this->all(node, nstart);
        return nplus;
    }

    RuleNode* Parser::until(RuleNode* node) {
        return this->pred([=](const char* b, const char* e, AnyT& val)->const char* {
            const char* c = b;
            do {
                auto m = node->match(c, e - c, 0);
                if (m->alter()) {
                    return c;
                }
                c++;
            } while (c != e);
            return nullptr;
            });
    }

    RuleNode* Parser::list(RuleNode* node, RuleNode* dem) {
        auto first = this->optional(node);
        auto tailItem = any(dem, node);
        auto tail = many(tailItem);
        return all(first, tail);
    }

    RuleNode* Parser::regex(const StrT& strRe) {
        std::regex re(strRe);
        return this->pred([=](const char* b, const char* e, AnyT& val)->const char* {
            std::smatch results;
            std::string toSearch(b, e);
            if (std::regex_search(toSearch, results, re)) {
                val = results.str();
                return b + results.position() + results.length();
            }
            return nullptr;
            });
    }

    RuleNode* Parser::identifier() {
        return regex("^[a-zA-Z_][a-zA-Z0-9_]*");
    }

    RuleNode* Parser::integer_() {
        return this->regex("^[-+]?\d+");
    }

    RuleNode* Parser::float_() {
        return this->regex("[-+]?\d*\.?\d+");
    }
};
