#pragma once
#include "../KParser.h"
#include <vector>
#include <unordered_map>

struct DSLNode {
    virtual ~DSLNode(){}
    virtual void visit( std::function<void(bool capture, DSLNode* node)> handle) {
        handle(false, this);
        handle(true, this);
    }
};

struct DSLID : public DSLNode {
    std::string name;
    DSLID(std::string name) :name(name) {};
};

struct DSLText : public DSLNode {
    std::string name;
    DSLText(std::string name) :name(name) {};
};


struct DSLRegex : public DSLNode {
    std::string name;
    DSLRegex(std::string name) :name(name) {};
};

struct DSLWrap : public DSLNode {
    DSLNode* node;
    DSLWrap(DSLNode* node) :node(node) {};
    void visit(std::function<void(bool capture, DSLNode* node)> handle) override {
        handle(false, this);
        node->visit(handle);
        handle(true, this);
    }
};

struct DSLMany : public DSLWrap {
    DSLMany(DSLNode* node) :DSLWrap(node) {};
};


struct DSLOption : public DSLWrap {
    DSLNode* node;
    DSLOption(DSLNode* node) :DSLWrap(node) {};
};

struct DSLChildren : public DSLNode {
    std::vector<DSLNode*> nodes;
    DSLChildren() {};
    void visit(std::function<void(bool capture, DSLNode* node)> handle) override {
        handle(false, this);
        for (auto& c : nodes) {
            c->visit(handle);
        }
        handle(true, this);
    }
};

struct DSLAny : public DSLChildren {
};

struct DSLAll : public DSLChildren {
};

struct DSLRule : public DSLWrap {
    std::string name;
    std::string evtName;
    DSLRule(DSLNode* node):DSLWrap(node){}
};

struct DSLRuleList : public DSLChildren {
};



struct DSLContext : public KParser::Parser {
    std::unordered_map <std::string, DSLNode*> nodeMap;
    std::unordered_map<KParser::Match*, size_t> matchMap;

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
    bool parse(std::string str);
};

DSLContext::DSLContext() {
    r_id = identifier();
    r_text = pred([&](const char* begin, const char* end, const char*& cb, const char*& ce, const char*& me) {
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

    r_regex = pred([&](const char* begin, const char* end, const char*& cb, const char*& ce, const char*& me) {
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
            }else if (st == ST::read_e) {
                if (*idx != 'e') {
                    goto fail;
                }
                st = ST::read_lp;
                idx++;
            }else if (st == ST::read_lp) {
                if (*idx != '`') {
                    goto fail;
                }
                st = ST::read_rp;
                idx++;
            }else if (st == ST::read_rp) {
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
    r_rule = all(r_id, optional(all("@", r_id)), "=", r_any, ";");
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
        auto& name = static_cast<DSLID*>(libany::any_cast<DSLNode*>(*b++))->name;
        std::string evtName;
        DSLNode* evtNode = libany::any_cast<DSLNode*>(*b++);
        if (&typeid(*evtNode) == &typeid(DSLID)) {
            evtName = static_cast<DSLID*>(evtNode)->name;
        }
        else {
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

bool DSLContext::parse(std::string strRuleList) {
    auto m = r_ruleList->parse(strRuleList);
    if (m == nullptr) {
        return false;
    }
    DSLNode** d = m->capture_s<DSLNode*>(0);
    DSLRuleList* rlist = (DSLRuleList*)*d;
    
    bool succ = true;
    // pass 0, collect all ID
    std::unordered_map<std::string, DSLRule*> idMap;
    auto hCheckID = [&](bool capture, DSLNode* n) {
        if(capture){
            if (&typeid(*n) == &typeid(DSLRule)) {
                DSLRule* rule = (DSLRule*)n;
                auto it = idMap.find(rule->name);
                if (it != idMap.end()) {
                    std::cerr << rule->name << " is duplicated " << std::endl;
                    succ = false;
                    return;
                }
                idMap.insert(it, std::make_pair(rule->name, rule));
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
            if (!wrap) return;
            DSLWrap* rule = dynamic_cast<DSLRule*>(n);
            // rule ≤ª”√ÃÊªª
            if (rule) return;

            auto* child = dynamic_cast<DSLID*>(wrap->node);
            if (!child) return;
            auto it = idMap.find(child->name);
            if (it != idMap.end()) {
                // replace
                succ = false;
                std::cout << "replace id " << child->name << std::endl;
                wrap->node = it->second->node;
            }
            else {
                std::cout << "invalid id of " << child->name << std::endl;
            }
        }
    };
    rlist->visit(hReplaceId);
    if (!succ) {
        return false;
    }
    

    // pass 2, check if rule is not DSLID
    // pass 3, change ID with rule node;
    
    return true;
}

