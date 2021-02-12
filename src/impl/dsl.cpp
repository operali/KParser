#include <iostream>
#include "common.h"
#include "dsl.h"
#include "util.h"

namespace KParser {

    DSLNode::DSLNode(DSLFactory* builder) :builder(builder) {
        builder->nodes.push_back(this);
    };

    void DSLNode::visit(std::function<void(bool sink, DSLNode* node)> handle) {
        handle(false, this);
        handle(true, this);
    }

    void DSLID::prepare(KParser::Parser& p) {
        if (name == "ID") {
            rule = p.identifier();
            rule->eval([&](auto& m, auto b, auto e) {
                auto s = m.str();
                return std::string(s);
                });
        }
        else if (name == "NUM") {
            rule = p.integer_();
            rule->eval([&](auto& m, auto b, auto e) {
                auto s = m.str();
                return int(std::atoi(s.c_str()));
                });
        }
        else if (name == "NONE") {
            rule = p.none();
        }
        else if (name == "COMMENT") {
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
            rule = p.any(commentRule1, commentRule2);
        }

        else if (name == "EOF") {
            rule = p.eof();
        }
        else {
            // rule = p.none();
            // lastError = std::string("unrecognized id of ") + name;
        }
    }


    void DSLText::prepare(KParser::Parser& p) {
        rule = p.str(std::string(name));
    }


    void DSLRegex::prepare(KParser::Parser& p) {
        rule = p.regex(name);
    }

    void DSLWrap::visit(std::function<void(bool sink, DSLNode* node)> handle) {
        if (visiting) {
            return;
        }
        visiting = true;
        handle(false, this);
        node->visit(handle);
        handle(true, this);
        visiting = false;
    }


    bool DSLMany::build(KParser::Parser& p) {
        if (!node->rule) {
            return false;
        }
        rule = p.many(node->rule);
        return true;
    }


    bool DSLMany1::build(KParser::Parser& p) {
        if (!node->rule) {
            return false;
        }
        rule = p.many1(node->rule);
        return true;
    }

    void DSLList::visit(std::function<void(bool sink, DSLNode* node)> handle) {
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
    bool DSLList::build(KParser::Parser& p) {
        if (!node->rule || !dem->rule) {
            return false;
        }
        rule = p.list(node->rule, dem->rule);
        return true;
    }

    bool DSLOption::build(KParser::Parser& p) {
        if (!node->rule) {
            return false;
        }
        rule = p.optional(node->rule);
        return true;
    }


    void DSLChildren::visit(std::function<void(bool sink, DSLNode* node)> handle) {
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

    void DSLAny::prepare(KParser::Parser& p) {
        rule = p.any();
    }
    bool DSLAny::build(KParser::Parser& p) {
        for (auto& c : nodes) {
            if (!c->rule) {
                std::cerr << "invalid rule of " << rule->toString() << std::endl;
                return false;
            }
            rule->add(c->rule);
        }
        return true;
    }

    void DSLAll::prepare(KParser::Parser& p) {
        rule = p.all();
    }
    bool DSLAll::build(KParser::Parser& p) {
        for (auto& c : nodes) {
            if (!c->rule) {
                std::cerr << "invalid rule of " << rule->toString() << std::endl;
                return false;
            }
            rule->add(c->rule);
        }
        return true;
    }

    bool DSLRule::build(KParser::Parser& p) {
        this->rule = node->rule;
        DSLContext* ctx = static_cast<DSLContext*>(builder);
        auto it = ctx->handleMap.find(this->name);
        if (it != ctx->handleMap.end()) {
            auto handle = it->second;
            this->rule->eval([=](KParser::Match& m, KParser::IT arg, KParser::IT noarg)->libany::any {
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

    void DSLContext::prepareEvaluation(const std::string& evtName, std::function<libany::any(KParser::Match& m, KParser::IT arg, KParser::IT noarg)> handle) {
        handleMap[evtName] = handle;
    }

    std::unique_ptr<KParser::Match> DSLContext::parse(const std::string& ruleName, const std::string& str) {
        auto it = idMap.find(ruleName);
        if (it != idMap.end()) {
            auto m = it->second->rule->parse(str);
            if (m == nullptr) {
                lastError = m_parser.errInfo();
            }
            return m;
        }
        return nullptr;
    }


    DSLContext::DSLContext() {
        KParser::Parser& p = m_parser;
        idMap.emplace(std::make_pair("ID", new DSLID{ this, "ID" , true }));
        idMap.emplace(std::make_pair("NUM", new DSLID{ this, "NUM", true }));
        idMap.emplace(std::make_pair("NONE", new DSLID{ this, "NONE", true }));
        idMap.emplace(std::make_pair("COMMENT", new DSLID{ this, "COMMENT", true }));
        idMap.emplace(std::make_pair("EOF", new DSLID{ this,"EOF", true }));

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
        r_list = p.all("[", r_item, r_item, "]");
        r_expr->add(r_many, r_many1, r_option, r_list, r_item);
        r_rule = p.all(r_id, "=", r_any, ";");
        r_ruleList = p.all(p.many1(r_rule), p.eof());

        r_id->eval([&](auto& m, auto b, auto e) { 
                return (DSLNode*)new DSLID{ this, m.str() };
            });
        r_regex->eval([&](auto& m, auto b, auto e) {
            std::string s = m.str();
            s = s.substr(1, s.length() - 2);
            return (DSLNode*)new DSLRegex{ this, s };
            });
        r_text->eval([&](auto& m, auto b, auto e) {
            auto s = m.str();
            std::string r;
            int rsz = 0;
            auto ret = parseCSTR(s.data(), s.length(), r, rsz);
            return (DSLNode*)new DSLText{ this, r };
            });

        r_any->eval([&](auto& m, auto b, auto e) {
            auto* node = new DSLAny(this);
            while (b != e) {
                node->nodes.push_back(libany::any_cast<DSLNode*>(*b++));
            }
            if (node->nodes.size() == 1) {
                DSLNode* n = node->nodes[0];
                return n;
            }
            return (DSLNode*)node;

            });
        r_all->eval([&](auto& m, auto b, auto e) {
            auto* node = new DSLAll(this);
            while (b != e) {
                node->nodes.push_back(libany::any_cast<DSLNode*>(*b++));
            }
            if (node->nodes.size() == 1) {
                DSLNode* n = node->nodes[0];
                return n;
            }
            return (DSLNode*)node;

            });
        r_many->eval([&](auto& m, auto b, auto e) {
            auto* node = new DSLMany(this, libany::any_cast<DSLNode*>(*b));
            return (DSLNode*)node;
            });
        r_many1->eval([&](auto& m, auto b, auto e) {
            auto* node = new DSLMany1(this, libany::any_cast<DSLNode*>(*b));
            return (DSLNode*)node;
            });
        r_list->eval([&](auto& m, auto b, auto e) {
            auto* item = libany::any_cast<DSLNode*>(*b++);
            auto* dem = libany::any_cast<DSLNode*>(*b++);
            auto* node = new DSLList(this, item, dem);
            return (DSLNode*)node;
            });
        r_option->eval([&](auto& m, auto b, auto e) {
            auto* node = new DSLOption(this, libany::any_cast<DSLNode*>(*b));
            return (DSLNode*)node;
            });
        r_rule->eval([&](auto& m, auto b, auto e) {
            DSLID* id = (DSLID*)libany::any_cast<DSLNode*>(*b++);
            DSLRule* r = new DSLRule(this, libany::any_cast<DSLNode*>(*b++));
            r->name = id->name;
            r->ruleLine = m.str();
            return (DSLNode*)r;
            });
        r_ruleList->eval([&](auto& m, auto b, auto e) {
            auto* node = new DSLRuleList(this);
            while (b != e) {
                node->nodes.push_back((DSLRule*)libany::any_cast<DSLNode*>(*b++));
            }
            return (DSLNode*)node;
            });
    }

    void DSLContext::prepareRules(std::string strRuleList) {
        m_strRule = strRuleList;
    }

    bool DSLContext::build() {
        auto m = r_ruleList->parse(m_strRule);
        if (m == nullptr) {
            lastError = m_parser.errInfo();
            std::cerr << lastError << std::endl;
            return false;
        }
        DSLNode** d = m->capture_s<DSLNode*>(0);
        DSLRuleList* rlist = (DSLRuleList*)*d;

        bool succ = true;
        
        // pass, collect p.all ID
        auto hCheckID = [&](bool sink, DSLNode* n) {
            if (sink) {
                if (&typeid(*n) == &typeid(DSLRule)) {
                    DSLRule* rule = (DSLRule*)n;
                    auto it = idMap.find(rule->name);
                    if (it != idMap.end()) {
                        std::cerr << rule->name << " is duplicated " << std::endl;
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
                if (id->is_prdefine) {
                    break;
                }
                auto it = idMap.find(id->name);
                if (it == idMap.end()) {
                    std::cerr << id->name << " is not defined " << std::endl;
                    lastError = id->name + " is not defined\n";
                    succ = false;
                    break;
                }
                node = it->second;
                if (node == kv.second) {
                    lastError = id->name + " is defined recursively\n";
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
                            auto* r = new DSLAny{ this };
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
                                lastError = std::string("invalid id of ") + id->name;
                            }
                        }
                        return;
                    }
                    auto* child = dynamic_cast<DSLID*>(wrap->node);
                    if (!child) return;
                    auto it = idMap.find(child->name);
                    if (it != idMap.end()) {
                        // replace
                        wrap->node = it->second;
                    }
                    else {
                        succ = false;
                        lastError = std::string("invalid id of ") + child->name;
                    }
                }
                else {
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
                                lastError = std::string("invalid id of ") + id->name;
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
                                lastError = std::string("invalid id of ") + id->name;
                            }
                        }
                    }
                    else {
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
                                    lastError = std::string("invalid id of ") + id->name;
                                }
                            }
                        }
                    }

                }
            }
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