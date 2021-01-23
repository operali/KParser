#pragma once
#include "../KParser.h"
#include <vector>
#include <unordered_map>


struct DSLNode {
    virtual ~DSLNode() {}
    bool visiting = false;
    virtual void visit(std::function<void(bool capture, DSLNode* node)> handle) {
        handle(false, this);
        handle(true, this);
    }
    KParser::Rule* rule = nullptr;

    virtual void prepare(KParser::Parser& p) {}
    virtual bool build(KParser::Parser& p) { return true; }
};

struct DSLID : public DSLNode {
    std::string name;
    DSLID(std::string name) :name(name) {};

    void prepare(KParser::Parser& p) override {
        if (rule) return;
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
        else if (name == "EOF") {
            rule = p.eof();
        }
        else {
            rule = p.none();
            std::cerr << "unrecognized ID: " << name << std::endl;
        }
    }
};

struct DSLText : public DSLNode {
    std::string name;
    DSLText(std::string name) :name(name) {};
    void prepare(KParser::Parser& p) override {
        if (rule) return;
        rule = p.str(std::string(name));
    }
};


struct DSLRegex : public DSLNode {
    std::string name;
    DSLRegex(std::string name) :name(name) {};
    void prepare(KParser::Parser& p) override {
        if (rule) return;
        rule = p.regex(name);
    }
};

struct DSLWrap : public DSLNode {
    DSLNode* node;
    DSLWrap(DSLNode* node) :node(node) {};
    void visit(std::function<void(bool capture, DSLNode* node)> handle) override {
        if (visiting) {
            return;
        }
        visiting = true;
        handle(false, this);
        node->visit(handle);
        handle(true, this);
        visiting = false;
    }
};

struct DSLMany : public DSLWrap {
    DSLMany(DSLNode* node) :DSLWrap(node) {};

    bool build(KParser::Parser& p) override {
        if (!node->rule) {
            return false;
        }
        rule = p.many(node->rule);
        return true;
    }
};


struct DSLOption : public DSLWrap {
    DSLOption(DSLNode* node) :DSLWrap(node) {};
    bool build(KParser::Parser& p) override {
        if (!node->rule) {
            return false;
        }
        rule = p.optional(node->rule);
        return true;
    }
};

struct DSLChildren : public DSLNode {
    std::vector<DSLNode*> nodes;
    DSLChildren() {};
    void visit(std::function<void(bool capture, DSLNode* node)> handle) override {
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
};

struct DSLAny : public DSLChildren {
    bool build(KParser::Parser& p) override {
        rule = p.any();
        for (auto& c : nodes) {
            if (!c->rule) {
                std::cerr << "invalid rule of " << rule->toString() << std::endl;
                return false;
            }
            rule->add(c->rule);
        }
        return true;
    }
};

struct DSLAll : public DSLChildren {
    bool build(KParser::Parser& p) override {
        rule = p.all();
        for (auto& c : nodes) {
            if (!c->rule) {
                std::cerr << "invalid rule of " << rule->toString() << std::endl;
                return false;
            }
            rule->add(c->rule);
        }
        return true;
    }
};

struct DSLRule : public DSLWrap {
    std::string name;
    std::string evtName;
    DSLRule(DSLNode* node) :DSLWrap(node) {}
    bool build(KParser::Parser& p) override;
};

struct DSLRuleList : public DSLChildren {
};



struct DSLContext : public KParser::Parser {
    std::unordered_map <std::string, DSLNode*> idMap;
    std::unordered_map <std::string, std::function<libany::any(KParser::Match& m, KParser::IT arg, KParser::IT noarg)>> handleMap;

    KParser::Rule* r_id;
    KParser::Rule* r_text; // `id`
    KParser::Rule* r_regex; // r`id`
    KParser::Rule* r_item; //  id, tex, re
    KParser::Rule* r_group; //  `(` r_any `)`;
    KParser::Rule* r_option; // item?
    KParser::Rule* r_many; // item*
    KParser::Rule* r_rule; // `id@evt` `=` any `;`
    KParser::Rule* r_any; // seq(all , "|");
    KParser::Rule* r_all; // many1(item);
    KParser::Rule* r_ruleList; // 

    DSLContext();
    bool ruleOf(std::string str);
    void bind(std::string evtName, std::function<libany::any(KParser::Match& m, KParser::IT arg, KParser::IT noarg)> handle) {
        handleMap[evtName] = handle;
    }
    
    bool parse(std::string ruleName, std::string str) {
        auto it = idMap.find(ruleName);
        if (it != idMap.end()) {
            auto m = it->second->rule->parse(str);
            if (m == nullptr) {
                return false;
            }
            return true;
        }
    }
};

DSLContext::DSLContext() {
    idMap.emplace(std::make_pair("ID", new DSLID{ "ID" }));
    idMap.emplace(std::make_pair("NUM", new DSLID{ "NUM" }));
    idMap.emplace(std::make_pair("NONE", new DSLID{ "NONE" }));
    idMap.emplace(std::make_pair("EOF", new DSLID{ "EOF" }));

    r_id = identifier();
    r_text = custom([&](const char* begin, const char* end, const char*& cb, const char*& ce, const char*& me) {
        auto* idx = begin;
        if (idx == end) {
            cb = ce = me = nullptr;
            return;
        }
        if (*idx++ != '`') {
            cb = ce = me = nullptr;
            return;
        }
        if (idx == end) {
            cb = ce = me = nullptr;
            return;
        }
        while (*idx++ != '`') {
            if (idx == end) {
                cb = ce = me = nullptr;
                return;
            }
        }
        me = idx;
        cb = begin + 1;
        ce = idx - 1;
        });

    r_regex = custom([&](const char* begin, const char* end, const char*& cb, const char*& ce, const char*& me) {
        auto* idx = begin;
        if (idx == end) {
            cb = ce = me = nullptr;
            return;
        }
        enum class ST {
            read_r = 0,
            read_e,
            read_lp,
            read_rp,
        };
        ST st = ST::read_r;

        while (true)
        {
            if (idx == end) {
                goto fail;
            }
            if (st == ST::read_r) {
                if (*idx != 'r') {
                    goto fail;
                }
                st = ST::read_e;
                idx++;
            }
            else if (st == ST::read_e) {
                if (*idx != 'e') {
                    goto fail;
                }
                st = ST::read_lp;
                idx++;
            }
            else if (st == ST::read_lp) {
                if (*idx != '`') {
                    goto fail;
                }
                st = ST::read_rp;
                idx++;
            }
            else if (st == ST::read_rp) {
                if (*idx++ == '`') {
                    goto succ;
                }
            }
        }
    fail:
        cb = ce = me = nullptr;
        return;
    succ:
        me = idx;
        cb = begin + 3;
        ce = idx - 1;
        });
    r_item = any();

    auto* r_expr = any();
    r_all = many1(r_expr);
    r_any = list(r_all, "|");
    r_group = all("(", r_any, ")");
    r_item->add(r_regex, r_text, r_id, r_group);
    r_option = all(r_item, "?");
    r_many = all(r_item, "*");
    r_expr->add(r_many, r_option, r_item);
    auto* strRule = identifier();
    strRule->eval([&](auto& m, auto b, auto e) {return m.str(); });
    r_rule = all(strRule, optional(all("@", strRule)), "=", r_any, ";");
    r_ruleList = many1(r_rule);

    r_id->eval([&](auto& m, auto b, auto e) {return (DSLNode*)new DSLID{ m.str() }; });
    r_regex->eval([&](auto& m, auto b, auto e) {return (DSLNode*)new DSLRegex{ m.str() }; });
    r_text->eval([&](auto& m, auto b, auto e) {return (DSLNode*)new DSLText{ m.str() }; });
    r_any->eval([&](auto& m, auto b, auto e) {
        auto* node = new DSLAny();
        while (b != e) {
            node->nodes.push_back(libany::any_cast<DSLNode*>(*b++));
        }
        if (node->nodes.size() == 1) {
            DSLNode* n = node->nodes[0];
            delete node;
            return n;
        }
        return (DSLNode*)node;

        });
    r_all->eval([&](auto& m, auto b, auto e) {
        auto* node = new DSLAll();
        while (b != e) {
            node->nodes.push_back(libany::any_cast<DSLNode*>(*b++));
        }
        if (node->nodes.size() == 1) {
            DSLNode* n = node->nodes[0];
            delete node;
            return n;
        }
        return (DSLNode*)node;

        });
    r_many->eval([&](auto& m, auto b, auto e) {
        auto* node = new DSLMany(libany::any_cast<DSLNode*>(*b));
        return (DSLNode*)node;
        });
    r_option->eval([&](auto& m, auto b, auto e) {
        auto* node = new DSLOption(libany::any_cast<DSLNode*>(*b));
        return (DSLNode*)node;
        });
    r_rule->eval([&](auto& m, auto b, auto e) {
        std::string name = libany::any_cast<std::string>(*b++);
        std::string evtName;
        try {
            evtName = libany::any_cast<std::string>(*b++);
        }
        catch (libany::bad_any_cast& ex) {
            b--;
        }

        DSLRule* r = new DSLRule(libany::any_cast<DSLNode*>(*b++));
        r->name = name;
        r->evtName = evtName;
        return (DSLNode*)r;
        });
    r_ruleList->eval([&](auto& m, auto b, auto e) {
        auto* node = new DSLRuleList();
        while (b != e) {
            node->nodes.push_back((DSLRule*)libany::any_cast<DSLNode*>(*b++));
        }
        return (DSLNode*)node;
        });
}

bool DSLContext::ruleOf(std::string strRuleList) {
    auto m = r_ruleList->parse(strRuleList);
    if (m == nullptr) {
        return false;
    }
    DSLNode** d = m->capture_s<DSLNode*>(0);
    DSLRuleList* rlist = (DSLRuleList*)*d;

    bool succ = true;
    // pass 0, collect all ID

    auto hCheckID = [&](bool capture, DSLNode* n) {
        if (capture) {
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

    // pass 1, check if ID is exist & replace
    auto hReplaceId = [&](bool capture, DSLNode* n) {
        if (capture) {
            DSLWrap* wrap = dynamic_cast<DSLWrap*>(n);
            if (wrap) {
                DSLRule* rule = dynamic_cast<DSLRule*>(n);
                // rule
                if (rule) {
                    return;
                }
                auto* child = dynamic_cast<DSLID*>(wrap->node);
                if (!child) return;
                auto it = idMap.find(child->name);
                if (it != idMap.end()) {
                    // replace
                    std::cout << "replace id " << child->name << std::endl;
                    wrap->node = it->second;
                }
                else {
                    succ = false;
                    std::cout << "invalid id of " << child->name << std::endl;
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
                            std::cout << "replace id " << id->name << std::endl;
                            children[i] = it->second;
                        }
                        else {
                            succ = false;
                            std::cout << "invalid id of " << id->name << std::endl;
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
    // pass , rule with evtName to replace by any
    auto hReplaceEvtRule = [&](bool capture, DSLNode* n) {
        if (capture) {
            DSLWrap* wrap = dynamic_cast<DSLWrap*>(n);
            if (wrap) {
                DSLRule* rule = dynamic_cast<DSLRule*>(n);
                if (rule) {
                    // wrap with any if this rule has evt
                    if (rule->evtName != "") {
                        auto* n = rule->node;
                        auto* r = new DSLAny{};
                        r->nodes.push_back(n);
                        rule->node = r;
                    }
                    idMap[rule->name] = rule;
                }
            }
        }
    };
    rlist->visit(hReplaceEvtRule);
    if (!succ) {
        return false;
    }
    // pass , check left recursive

    // pass , build RULES
    auto hBuild = [&](bool capture, DSLNode* n) {
        if (!capture) {
            n->prepare(*this);
            return;
        }
        else {
            auto r = n->build(*this);
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

bool DSLRule::build(KParser::Parser& p) {
    this->rule = node->rule;
    if (evtName != "") {
        this->rule->eval([&](KParser::Match& m, KParser::IT arg, KParser::IT noarg)->libany::any {
            DSLContext* ctx = static_cast<DSLContext*>(&p);
            auto it = ctx->handleMap.find(evtName);
            if (it == ctx->handleMap.end()) {
                return 0;
            }
            return it->second(m, arg, noarg);
            });
    }
    return true;
}
