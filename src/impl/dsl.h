#pragma once
#include "../KParser.h"
#include <vector>
#include <unordered_map>

struct DSLNode;
struct DSLFactory {
    std::vector<DSLNode*> nodes;
    ~DSLFactory();
};

struct DSLNode {
    DSLNode(DSLFactory* builder) {
        builder->nodes.push_back(this);
    };

    virtual ~DSLNode() {}
    bool visiting = false;
    virtual void visit(std::function<void(bool sink, DSLNode * node)> handle) {
        handle(false, this);
        handle(true, this);
    }
    KParser::Rule* rule = nullptr;
    bool haveBuild = false;
    virtual void prepare(KParser::Parser& p) {}
    virtual bool build(KParser::Parser& p) { return true; }
};

struct DSLID : public DSLNode {
    std::string name;
    DSLID(DSLFactory* builder, std::string name) :DSLNode(builder), name(name) {};

    void prepare(KParser::Parser& p) override {
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
            // rule = p.none();
            // lastError = std::string("unrecognized id of ") + name;
        }
    }
};

struct DSLText : public DSLNode {
    std::string name;
    DSLText(DSLFactory* builder, std::string name) :DSLNode(builder), name(name) {};
    void prepare(KParser::Parser& p) override {
        rule = p.str(std::string(name));
    }
};


struct DSLRegex : public DSLNode {
    std::string name;
    DSLRegex(DSLFactory* builder, std::string name) :DSLNode(builder), name(name) {};
    void prepare(KParser::Parser& p) override {
        rule = p.regex(name);
    }
};

struct DSLWrap : public DSLNode {
    DSLNode* node;
    DSLWrap(DSLFactory* builder, DSLNode* node) :DSLNode(builder), node(node) {};
    void visit(std::function<void(bool sink, DSLNode * node)> handle) override {
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
    DSLMany(DSLFactory* builder, DSLNode* node) :DSLWrap(builder, node) {};

    bool build(KParser::Parser& p) override {
        if (!node->rule) {
            return false;
        }
        rule = p.many(node->rule);
        return true;
    }
};

struct DSLMany1 : public DSLWrap {
    DSLMany1(DSLFactory* builder, DSLNode* node) :DSLWrap(builder, node) {};

    bool build(KParser::Parser& p) override {
        if (!node->rule) {
            return false;
        }
        rule = p.many1(node->rule);
        return true;
    }
};

struct DSLList : public DSLNode {
    DSLNode* node;
    DSLNode* dem;
    DSLList(DSLFactory* builder, DSLNode* node, DSLNode* dem) :DSLNode(builder), node(node), dem(dem){};

    void visit(std::function<void(bool sink, DSLNode * node)> handle) override {
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
    bool build(KParser::Parser& p) override {
        if (!node->rule || !dem->rule) {
            return false;
        }
        rule = p.list(node->rule, dem->rule);
        return true;
    }
};

struct DSLOption : public DSLWrap {
    DSLOption(DSLFactory* builder, DSLNode* node) :DSLWrap(builder, node) {};
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
    DSLChildren(DSLFactory* builder) :DSLNode(builder) {};
    
    void visit(std::function<void(bool sink, DSLNode * node)> handle) override {
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
    DSLAny(DSLFactory* builder) :DSLChildren(builder) {
    };
    void prepare(KParser::Parser& p) override {
        rule = p.any();
    }
    bool build(KParser::Parser& p) override {
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
    DSLAll(DSLFactory* builder) :DSLChildren(builder) {};
    void prepare(KParser::Parser& p) override {
        rule = p.all();
    }
    bool build(KParser::Parser& p) override {
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
    std::string ruleLine;
    DSLRule(DSLFactory* builder, DSLNode* node) :DSLWrap(builder, node) {}
    bool build(KParser::Parser& p) override;
};

struct DSLRuleList : public DSLChildren {
    DSLRuleList(DSLFactory* builder) :DSLChildren(builder) {};
};



struct DSLContext : public KParser::Parser {
    DSLFactory builder;
    std::string lastError;
    std::unordered_map <std::string, DSLNode*> idMap;
    std::unordered_map <std::string, std::function<libany::any(KParser::Match & m, KParser::IT arg, KParser::IT noarg)>> handleMap;

    KParser::Rule* r_id;
    KParser::Rule* r_text; // `id`
    KParser::Rule* r_regex; // r`id`
    KParser::Rule* r_item; //  id, tex, re
    KParser::Rule* r_group; //  `(` r_any `)`;
    KParser::Rule* r_option; // item?
    KParser::Rule* r_many; // item*
    KParser::Rule* r_many1; // item+
    KParser::Rule* r_list; // [k, x]
    KParser::Rule* r_rule; // `id` `=` any `;`
    KParser::Rule* r_any; // seq(all , "|");
    KParser::Rule* r_all; // many1(item);
    KParser::Rule* r_ruleList; // 

    DSLContext();
    bool ruleOf(std::string str);
    void bind(std::string evtName, std::function<libany::any(KParser::Match & m, KParser::IT arg, KParser::IT noarg)> handle) {
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
        return false;
    }
};

DSLContext::DSLContext() {
    idMap.emplace(std::make_pair("ID", new DSLID{ &builder, "ID" }));
    idMap.emplace(std::make_pair("NUM", new DSLID{ &builder, "NUM" }));
    idMap.emplace(std::make_pair("NONE", new DSLID{ &builder, "NONE" }));
    idMap.emplace(std::make_pair("EOF", new DSLID{ &builder,"EOF" }));

    r_id = identifier();
    r_text = custom([&](const char* begin, const char* end)->const char* {
        auto* idx = begin;
        if (idx == end) {
            return nullptr;
        }
        if (*idx++ != '`') {
            return nullptr;
        }
        if (idx == end) {
            return nullptr;
        }
        while (*idx++ != '`') {
            if (idx == end) {
                return nullptr;
            }
        }
        return idx;
        });

    r_regex = custom([&](const char* begin, const char* end)->const char* {
        auto* idx = begin;
        if (idx == end) {
            return nullptr;
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
        return nullptr;
    succ:
        return idx;
        });
    r_item = any();

    auto* r_expr = any();
    r_all = many1(r_expr);
    r_any = list(r_all, "|");
    r_group = all("(", r_any, ")");
    r_item->add(r_regex, r_text, r_id, r_group);
    r_option = all(r_item, "?");
    r_many = all(r_item, "*");
    r_many1 = all(r_item, "+");
    r_list = all("[", r_item, r_item, "]");
    r_expr->add(r_many, r_many1, r_option, r_list, r_item);
    auto* strRule = identifier();
    strRule->eval([&](auto& m, auto b, auto e) {return m.str(); });
    r_rule = all(strRule, optional(all("@", strRule)), "=", r_any, ";");
    r_ruleList = all(many1(r_rule), eof());

    r_id->eval([&](auto& m, auto b, auto e) {return (DSLNode*)new DSLID{ &builder, m.str() }; });
    r_regex->eval([&](auto& m, auto b, auto e) {
        std::string s = m.str();
        s = s.substr(3, s.length() - 4);
        return (DSLNode*)new DSLRegex{ &builder, s }; 
    });
    r_text->eval([&](auto& m, auto b, auto e) {
        std::string s = m.str();
        s = s.substr(1, s.length() - 2);
        return (DSLNode*)new DSLText{ &builder, s }; 
    });

    r_any->eval([&](auto& m, auto b, auto e) {
        auto* node = new DSLAny(&builder);
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
        auto* node = new DSLAll(&builder);
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
        auto* node = new DSLMany(&builder, libany::any_cast<DSLNode*>(*b));
        return (DSLNode*)node;
        });
    r_many1->eval([&](auto& m, auto b, auto e) {
        auto* node = new DSLMany1(&builder, libany::any_cast<DSLNode*>(*b));
        return (DSLNode*)node;
        });
    r_list->eval([&](auto& m, auto b, auto e) {
        auto* item = libany::any_cast<DSLNode*>(*b++);
        auto* dem = libany::any_cast<DSLNode*>(*b++);
        auto* node = new DSLList(&builder, item, dem);
        return (DSLNode*)node;
        });
    r_option->eval([&](auto& m, auto b, auto e) {
        auto* node = new DSLOption(&builder, libany::any_cast<DSLNode*>(*b));
        return (DSLNode*)node;
        });
    r_rule->eval([&](auto& m, auto b, auto e) {
        std::string name = libany::any_cast<std::string>(*b++);
        std::string evtName;
        try {
            evtName = libany::any_cast<std::string>(*b++);
        }
        catch (libany::bad_any_cast & ex) {
            b--;
        }

        DSLRule* r = new DSLRule(&builder, libany::any_cast<DSLNode*>(*b++));
        r->name = name;
        r->evtName = evtName;
        r->ruleLine = m.str();
        return (DSLNode*)r;
        });
    r_ruleList->eval([&](auto& m, auto b, auto e) {
        auto* node = new DSLRuleList(&builder);
        while (b != e) {
            node->nodes.push_back((DSLRule*)libany::any_cast<DSLNode*>(*b++));
        }
        return (DSLNode*)node;
        });
}

bool DSLContext::ruleOf(std::string strRuleList) {
    auto m = r_ruleList->parse(strRuleList);
    if (m == nullptr) {
        lastError = this->errInfo();
        return false;
    }
    DSLNode** d = m->capture_s<DSLNode*>(0);
    DSLRuleList* rlist = (DSLRuleList*)*d;

    bool succ = true;
    // pass 0, collect all ID

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

    // pass , rule with evtName to replace by any
    auto hReplaceEvtRule = [&](bool sink, DSLNode* n) {
        if (sink) {
            DSLRule* rule = dynamic_cast<DSLRule*>(n);
            if (rule) {
                // wrap with any if this rule has evt & equal id
                if (rule->evtName != "") {
                    auto* rn = rule->node;
                    auto* id = dynamic_cast<DSLID*>(rn);
                    if (id) {
                        auto* r = new DSLAny{ &builder };
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

    // pass 1, check if ID is exist & replace
    auto hReplaceId = [&](bool sink, DSLNode* n) {
        if (sink) {
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
                    wrap->node = it->second;
                }
                else {
                    succ = false;
                    lastError = std::string("invalid id of ") + child->name;
                }
            } 
            else {
                DSLList* list = dynamic_cast<DSLList*>(n);
                if (list) {
                    auto* id = dynamic_cast<DSLID*>(list->node);
                    if (id) {
                        auto it = idMap.find(id->name);
                        if (it != idMap.end()) {
                            // replace
                            list->node = it->second;
                        }
                        else {
                            succ = false;
                            lastError = std::string("invalid id of ") + id->name;
                        }
                    }
                    id = dynamic_cast<DSLID*>(list->dem);
                    if (id) {
                        auto it = idMap.find(id->name);
                        if (it != idMap.end()) {
                            // replace
                            list->dem = it->second;
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
            n->prepare(*this);
            return;
        }
        else {
            auto r = n->build(*this);
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


DSLFactory::~DSLFactory() {
    for (auto* node : nodes) {
        delete node;
    }
}