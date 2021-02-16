#include <iostream>
#include "common.h"
#include "dsl.h"
#include "util.h"
#include "../KParser.h"

namespace KLib42 {

    DSLNode::DSLNode(DSLFactory* builder, KUnique<IRange> range) :builder(builder), range(range){
        builder->nodes.push_back(this);
    };

    void DSLNode::visit(std::function<void(bool sink, DSLNode * node)> handle) {
        handle(false, this);
        handle(true, this);
    }

    void DSLID::prepare(KLib42::Parser& p) {
        if (name == "ID") {
            rule = p.identifier();
            rule->eval([&](Match& m, IT b, IT e) {
                auto s = m.str();
                return std::string(s);
                });
        }
        else if (name == "NUM") {
            rule = p.integer_();
            rule->eval([&](Match& m, IT b, IT e) {
                auto s = m.str();
                return int(std::atoi(s.c_str()));
                });
        }
        else if (name == "NONE") {
            rule = p.none();
        }
        else if (name == "EOF") {
            rule = p.eof();
        }
        else {
            // rule = p.none();
            // lastError = std::string("unrecognized id of ") + name;
        }
    }


    void DSLText::prepare(KLib42::Parser& p) {
        rule = p.str(std::string(name));
    }


    void DSLRegex::prepare(KLib42::Parser& p) {
        rule = p.regex(name);
    }

    void DSLWrap::visit(std::function<void(bool sink, DSLNode * node)> handle) {
        if (visiting) {
            return;
        }
        visiting = true;
        handle(false, this);
        node->visit(handle);
        handle(true, this);
        visiting = false;
    }


    bool DSLMany::build(KLib42::Parser& p) {
        if (!node->rule) {
            return false;
        }
        rule = p.many(node->rule);
        return true;
    }


    bool DSLMany1::build(KLib42::Parser& p) {
        if (!node->rule) {
            return false;
        }
        rule = p.many1(node->rule);
        return true;
    }

    void DSLList::visit(std::function<void(bool sink, DSLNode * node)> handle) {
        if (visiting) {
            return;
        }
        visiting = true;
        handle(false, this);
        node->visit(handle);
        dem->visit(handle);
        handle(true, this);
        visiting = false;
    }
    bool DSLList::build(KLib42::Parser& p) {
        if (!node->rule || !dem->rule) {
            return false;
        }
        rule = p.list(node->rule, dem->rule);
        return true;
    }

    bool DSLTill::build(Parser& p) {
        if (!node->rule) {
            return false;
        }
        rule = p.till(node->rule);
        return true;
    };

    bool DSLOption::build(KLib42::Parser& p) {
        if (!node->rule) {
            return false;
        }
        rule = p.optional(node->rule);
        return true;
    }


    void DSLChildren::visit(std::function<void(bool sink, DSLNode * node)> handle) {
        if (visiting) {
            return;
        }
        visiting = true;
        handle(false, this);
        for (auto& c : nodes) {
            c->visit(handle);
        }
        handle(true, this);
        visiting = false;
    }

    void DSLAny::prepare(KLib42::Parser& p) {
        rule = p.any();
    }
    bool DSLAny::build(KLib42::Parser& p) {
        for (auto& c : nodes) {
            if (!c->rule) {
                std::cerr << "invalid rule of " << rule->toString() << std::endl;
                return false;
            }
            rule->add(c->rule);
        }
        return true;
    }

    void DSLAll::prepare(KLib42::Parser& p) {
        rule = p.all();
    }
    bool DSLAll::build(KLib42::Parser& p) {
        for (auto& c : nodes) {
            if (!c->rule) {
                std::cerr << "invalid rule of " << rule->toString() << std::endl;
                return false;
            }
            rule->add(c->rule);
        }
        return true;
    }

    bool DSLRule::build(KLib42::Parser& p) {
        this->rule = node->rule;
        DSLContext* ctx = static_cast<DSLContext*>(builder);
        auto it = ctx->handleMap.find(this->name);
        if (it != ctx->handleMap.end()) {
            auto handle = it->second;
            this->rule->eval([=](KLib42::Match& m, KLib42::IT arg, KLib42::IT noarg)->KAny {
                return handle(m, arg, noarg);
                });
        }
        return true;
    }


    DSLFactory::~DSLFactory() {
        for (auto* node : nodes) {
            delete node;
        }
    }

    void DSLContext::prepareEvaluation(const std::string& evtName, std::function<KAny(KLib42::Match & m, KLib42::IT arg, KLib42::IT noarg)> handle) {
        handleMap[evtName] = handle;
    }

    KUnique<KLib42::Match> DSLContext::parse(const std::string& ruleName, const std::string& str) {
        auto it = idMap.find(ruleName);
        if (it != idMap.end()) {
            auto m = it->second->rule->parse(str);
            if (m.get() == nullptr) {
                lastError = m_parser.getErrInfo();
            }
            return m;
        }
        return nullptr;
    }
    static KUnique<IRange> getRange(Parser& p, Match& m) {
        auto loc = m.location();
        auto len = m.length();
        auto src = p.getSource();
        auto range = src->getRange(loc, loc + len);
        return range;
    }

    DSLContext::DSLContext() {
        KLib42::Parser& p = m_parser;
        idMap.emplace(std::make_pair("ID", new DSLID{ this, nullptr, "ID" , true }));
        idMap.emplace(std::make_pair("NUM", new DSLID{ this, nullptr, "NUM", true }));
        idMap.emplace(std::make_pair("NONE", new DSLID{ this, nullptr, "NONE", true }));
        idMap.emplace(std::make_pair("EOF", new DSLID{ this,nullptr, "EOF", true }));

        auto commentRule1 = p.custom([=](const char* b, const char* e)->const char* {
            if (*b++ != '/' || b == e) {
                return nullptr;
            }
            if (*b++ != '*' || b == e) {
                return nullptr;
            }
            bool tryEnd = false;
            for (; b != e; ++b) {
                if (tryEnd) {
                    if (*b == '/') {
                        return b + 1;
                    }
                    else {
                        tryEnd = false;
                    }
                }
                else {
                    if (*b == '*') {
                        tryEnd = true;
                    }
                }
            }
            return b;
            });
        auto commentRule2 = p.custom([=](const char* b, const char* e)->const char* {
            if (*b++ != '/' || b == e) {
                return nullptr;
            }
            if (*b++ != '/' || b == e) {
                return nullptr;
            }

            for (; b != e; ++b) {
                if (*b == '\n') {
                    return b;
                }
            }
            return b;
            });
        r_comment = p.any(commentRule1, commentRule2);

        r_id = p.regex(R"(^[a-zA-Z_][a-zA-Z0-9_]*)");
        r_text = p.custom([&](const char* begin, const char* end)->const char* {
            int rsize = 0;
            std::string r;
            auto res = parseCSTR(begin, end - begin, r, rsize);
            if (!res) {
                return nullptr;
            }
            return begin + rsize;
            });

        r_regex = p.custom([&](const char* begin, const char* end)->const char* {
            int len = 0;
            bool r = parseRegex(begin, end - begin, len);
            if (r) {
                return begin + len;
            }
            return nullptr;
            });

        r_item = p.any();

        auto* r_expr = p.any();
        r_all = p.many1(r_expr);
        r_any = p.list(r_all, "|");
        r_group = p.all("(", r_any, ")");
        r_item->add(r_regex, r_text, r_id, r_group);
        r_option = p.all(r_item, "?");
        r_many = p.all(r_item, "*");
        r_many1 = p.all(r_item, "+");
        r_till = p.all("...", r_item);
        r_list = p.all("[", r_item, r_item, "]");
        r_expr->add(r_till, r_many, r_many1, r_option, r_list, r_item);
        r_rule = p.all(p.many(r_comment), r_id, "=", r_any, ";", p.many(r_comment));
        r_ruleList = p.all(p.many1(r_rule), p.eof());


        r_id->eval([&](Match& m, IT b, IT e) {
            return (DSLNode*)new DSLID(this, getRange(p, m), m.str() );
            });
        r_regex->eval([&](Match& m, IT b, IT e) {
            std::string s = m.str();
            s = s.substr(1, s.length() - 2);
            return (DSLNode*)new DSLRegex{ this, getRange(p, m), s };
            });
        r_text->eval([&](Match& m, IT b, IT e) {
            auto s = m.str();
            std::string r;
            int rsz = 0;
            auto ret = parseCSTR(s.data(), s.length(), r, rsz);
            return (DSLNode*)new DSLText{ this, getRange(p, m), r };
            });

        r_any->eval([&](Match& m, IT b, IT e) {
            auto* node = new DSLAny(this, getRange(p, m));
            while (b != e) {
                node->nodes.push_back(*(*b++).template get<DSLNode*>());
            }
            if (node->nodes.size() == 1) {
                DSLNode* n = node->nodes[0];
                return n;
            }
            return (DSLNode*)node;

            });
        r_all->eval([&](Match& m, IT b, IT e) {
            auto* node = new DSLAll(this, getRange(p, m));
            while (b != e) {
                node->nodes.push_back(*(*b++).template get<DSLNode*>());
            }
            if (node->nodes.size() == 1) {
                DSLNode* n = node->nodes[0];
                return n;
            }
            return (DSLNode*)node;

            });
        r_many->eval([&](Match& m, IT b, IT e) {
            auto* node = new DSLMany(this, getRange(p, m), *(*b).template get<DSLNode*>());
            return (DSLNode*)node;
            });
        r_many1->eval([&](Match& m, IT b, IT e) {
            auto* node = new DSLMany1(this, getRange(p, m), *(*b).template get<DSLNode*>());
            return (DSLNode*)node;
            });
        r_list->eval([&](Match& m, IT b, IT e) {
            auto* item = *(*b++).template get<DSLNode*>();
            auto* dem = *(*b++).template get<DSLNode*>();
            auto* node = new DSLList(this, getRange(p, m), item, dem);
            return (DSLNode*)node;
            });
        r_till->eval([&](Match& m, IT b, IT e) {
            auto* item = *(*b++).template get<DSLNode*>();
            auto* node = new DSLTill(this, getRange(p, m), item);
            return (DSLNode*)node;
            });
        r_option->eval([&](Match& m, IT b, IT e) {
            auto* node = new DSLOption(this, getRange(p, m), *(*b).template get<DSLNode*>());
            return (DSLNode*)node;
            });
        r_rule->eval([&](Match& m, IT b, IT e) {
            DSLID* id = (DSLID*)*(*b++).template get<DSLNode*>();
            DSLRule* r = new DSLRule(this, getRange(p, m), *(*b++).template get<DSLNode*>());
            r->name = id->name;
            r->id = id;
            r->ruleLine = m.str();
            return (DSLNode*)r;
            });
        r_ruleList->eval([&](Match& m, IT b, IT e) {
            auto* node = new DSLRuleList(this, getRange(p, m));
            while (b != e) {
                node->nodes.push_back((DSLRule*)*(*b++).template get<DSLNode*>());
            }
            return (DSLNode*)node;
            });
    }

    void DSLContext::prepareRules(std::string strRuleList) {
        m_strRule = strRuleList;
    }

    bool DSLContext::build() {
        auto m = r_ruleList->parse(m_strRule);
        if (!m) {
            lastError = m_parser.getErrInfo();
            std::cerr << lastError << std::endl;
            return false;
        }
        DSLNode** d = m->capture<DSLNode*>(0);
        DSLRuleList* rlist = (DSLRuleList*)*d;

        bool succ = true;

        // pass, collect p.all ID
        auto hCheckID = [&](bool sink, DSLNode* n) {
            if (sink) {
                if (&typeid(*n) == &typeid(DSLRule)) {
                    DSLRule* rule = (DSLRule*)n;
                    auto it = idMap.find(rule->name);
                    if (it != idMap.end()) {
                        lastError = KShared<KError>(new RedefinedIDError(this->m_parser.getSource(), rule->id));
                        succ = false;
                        return;
                    }
                    idMap.insert(it, std::make_pair(rule->name, rule->node));
                }
            }
        };
        rlist->visit(hCheckID);
        if (!succ) {
            return false;
        }

        // pass, check all id
        std::vector<std::pair<std::string, DSLNode*>> changes;
        for (auto& kv : idMap) {
            auto& name = kv.first;
            auto* node = kv.second;

            while (true) {
                auto* id = dynamic_cast<DSLID*>(node);
                if (id == nullptr) {
                    break;
                }
                if (id->isPreset) {
                    break;
                }
                auto it = idMap.find(id->name);
                if (it == idMap.end()) {
                    std::cerr << id->name << " is not defined " << std::endl;
                    // id->name + " is not defined\n";
                    lastError = KShared<KError>(new UndefinedIDError(this->m_parser.getSource(), id));
                    succ = false;
                    break;
                }
                node = it->second;
                if (node == kv.second) {
                    lastError = KShared<KError>(new RecursiveIDError(this->m_parser.getSource(), id));
                    // lastError = id->name + " is defined recursively\n";
                    succ = false;
                    break;
                }
            }
            if (!succ) {
                return false;
            }
            if (node != kv.second) {
                changes.push_back(std::make_pair(kv.first, node));
            }
        }
        if (!succ) {
            return false;
        }
        for (auto& kv : changes) {
            idMap[kv.first] = kv.second;
        }


        // pass , rule with evtName to replace by any
        auto hReplaceEvtRule = [&](bool sink, DSLNode* n) {
            if (sink) {
                DSLRule* rule = dynamic_cast<DSLRule*>(n);
                if (rule) {
                    // wrap with any if this rule has evt & equal id
                    auto it = handleMap.find(rule->name);
                    if (it != handleMap.end()) {
                        auto* rn = rule->node;
                        auto* id = dynamic_cast<DSLID*>(rn);
                        if (id) {
                            auto* r = new DSLAny{ this, id->range };
                            r->nodes.push_back(rn);
                            rule->node = r;
                            idMap[rule->name] = r;
                        }
                    }
                }
            }
        };
        rlist->visit(hReplaceEvtRule);
        if (!succ) {
            return false;
        }

        // pass, check if ID is exist & replace
        auto hReplaceId = [&](bool sink, DSLNode* n) {
            if (sink) {
                DSLWrap* wrap = dynamic_cast<DSLWrap*>(n);
                if (wrap) {
                    DSLRule* rule = dynamic_cast<DSLRule*>(n);
                    // rule
                    if (rule) {
                        {
                            DSLID* id = dynamic_cast<DSLID*>(rule->node);
                            if (!id) return;
                            auto it = idMap.find(id->name);
                            if (it != idMap.end()) {
                                // replace
                                rule->node = it->second;
                            }
                            else {
                                succ = false;
                                // std::string("invalid id of ") + id->name;
                                lastError = KShared<KError>(new UndefinedIDError(this->m_parser.getSource(), id));
                            }
                        }
                        return;
                    }
                    auto* id = dynamic_cast<DSLID*>(wrap->node);
                    if (!id) return;
                    auto it = idMap.find(id->name);
                    if (it != idMap.end()) {
                        // replace
                        wrap->node = it->second;
                    }
                    else {
                        succ = false;
                        lastError = KShared<KError>(new UndefinedIDError(this->m_parser.getSource(), id));
                        // lastError = std::string("invalid id of ") + child->name;
                        return;
                    }
                }
                DSLID* idNode = dynamic_cast<DSLID*>(n);
                if (idNode) {
                    auto it = idMap.find(idNode->name);
                    if (it == idMap.end()) {
                        succ = false;
                        lastError = KShared<KError>(new UndefinedIDError(this->m_parser.getSource(), idNode));
                        // lastError = std::string("invalid id of ") + id->name;
                    }
                    return;
                }
                DSLList* plist = dynamic_cast<DSLList*>(n);
                if (plist) {
                    auto* id = dynamic_cast<DSLID*>(plist->node);
                    if (id) {
                        auto it = idMap.find(id->name);
                        if (it != idMap.end()) {
                            // replace
                            plist->node = it->second;
                        }
                        else {
                            succ = false;
                            lastError = KShared<KError>(new UndefinedIDError(this->m_parser.getSource(), id));
                            // lastError = std::string("invalid id of ") + id->name;
                        }
                    }
                    id = dynamic_cast<DSLID*>(plist->dem);
                    if (id) {
                        auto it = idMap.find(id->name);
                        if (it != idMap.end()) {
                            // replace
                            plist->dem = it->second;
                        }
                        else {
                            succ = false;
                            lastError = KShared<KError>(new UndefinedIDError(this->m_parser.getSource(), id));
                            // lastError = std::string("invalid id of ") + id->name;
                        }
                    }
                    return;
                }
                DSLChildren* hasChildren = dynamic_cast<DSLChildren*>(n);
                if (hasChildren) {
                    DSLRuleList* rules = dynamic_cast<DSLRuleList*>(n);
                    if (rules) {
                        return;
                    }
                    auto& children = hasChildren->nodes;
                    for (size_t i = 0; i < children.size(); ++i) {
                        auto& c = children.at(i);
                        auto* id = dynamic_cast<DSLID*>(c);
                        if (!id) continue;
                        auto it = idMap.find(id->name);
                        if (it != idMap.end()) {
                            // replace
                            children[i] = it->second;
                        }
                        else {
                            succ = false;
                            lastError = KShared<KError>(new UndefinedIDError(this->m_parser.getSource(), id));
                            // lastError = std::string("invalid id of ") + id->name;
                        }
                    }
                }
            };
        };
            rlist->visit(hReplaceId);
            if (!succ) {
                return false;
            }

            // pass , check left recursive

            // pass , build RULES
            auto hBuild = [&](bool sink, DSLNode* n) {
                if (n->haveBuild) {
                    return;
                }
                if (!sink) {
                    n->prepare(m_parser);
                    return;
                }
                else {
                    auto r = n->build(m_parser);
                    n->haveBuild = true;
                    if (!r) {
                        succ = false;
                    }
                    return;
                }
            };
            rlist->visit(hBuild);
            if (!succ) {
                return false;
            }
            return true;
        }
    }