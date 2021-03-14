// author: operali
// desc: Domain special language of ENBF

#include <iostream>
#include "common.h"
#include "../kparser.h"
#include "./dsl.h"
#include "./util.h"
#include "./impl.h"

namespace KLib42 {

    DSLNode::DSLNode(KUnique<IRange> range) :range(range) {
    };

    void DSLNode::visit(std::function<void(bool sink, DSLNode* node)> handle) {
        handle(false, this);
        handle(true, this);
    }

    bool DSLCUT::build(Parser& p) {
        rule = p.cut();
        return true;
    };

    void DSLText::prepare(KLib42::Parser& p) {
        rule = p.str(std::string(name));
    }

    
    void DSLRegex::prepare(KLib42::Parser& p) {
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
        return true;
    }

    void DSLContext::prepareCapture(const std::string& evtName, CaptureT&& handle) {
        handleMap[evtName] = handle;
    }

    KUnique<KLib42::Match> DSLContext::parse(const std::string& ruleName, const std::string& str) {
        auto it = idMap.find(ruleName);
        if (it != idMap.end()) {
            auto m = it->second->rule->parse(str);
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

    DSLContext::~DSLContext() {
        for (auto* node : m_nodes) {
            delete node;
        }
    }

    DSLContext::DSLContext() {
        KLib42::Parser& p = m_ruleParser;
        
        prepareConstant("ID", [=](const char* b, const char* e)->const char* {
            int len;
            if (parseIdentifier(b, e - b, len)) {
                return b + len;
            }
            else {
                return nullptr;
            }
            });
        prepareCapture("ID", [](Match& m, auto b, auto e) {
            return m.str();
            });
        prepareConstant("NUM", [=](const char* b, const char* e)->const char* {
            double ret;
            int len;
            if (parseFloat(b, e - b, ret, len)) {
                return b + len;
            }
            else {
                return nullptr;
            }
            });
        prepareCapture("NUM", [](Match& m, auto b, auto e) {
            auto s = m.str();
            const char* d = s.data();
            double ret;
            int len;
            parseFloat(d, s.length(), ret, len);
            return ret;
            });
        prepareConstant("NONE", [](const char* b, const char* e)->const char* {
            return b;
            });
        prepareConstant("EOF", [](const char* b, const char* e)->const char* {
            if (b == e) {
                return b;
            }
            return nullptr;
            });
        

        auto commentRule1 = [=](const char* b, const char* e)->const char* {
            // match left
            if (*b++ != '/' || b == e) {
                return nullptr;
            }
            if (*b++ != '*' || b == e) {
                return nullptr;
            }

            bool matchRight = false;
            for (; b != e; ++b) {
                if (matchRight) {
                    if (*b == '/') {
                        return b + 1;
                    }
                    else {
                        matchRight = false;
                    }
                }
                else {
                    if (*b == '*') {
                        matchRight = true;
                    }
                }
            }
            return b;
        };
        auto commentRule2 = [=](const char* b, const char* e)->const char* {
            auto r = commentRule1(b, e);
            if (r) {
                return r;
            }
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
        };
        p.setSkippedRule(commentRule2);

        r_id = p.identifier();
        r_cut = p.str("!");
        r_text = p.custom([&](const char* begin, const char* end)->const char* {
            int len = 0;
            std::string r;
            auto res = parseCSTR(begin, end - begin, r, len);
            if (!res) {
                return nullptr;
            }
            return begin + len;
            });

        r_regex = p.custom([&](const char* begin, const char* end)->const char* {
            int len = 0;
            bool res = parseRegex(begin, end - begin, len);
            if (!res) {
                return nullptr;
            }
            return begin + len;
            });

        r_item = p.any();

        auto* r_expr = p.any();
        r_all = p.many1(r_expr);
        r_any = p.list(r_all, "|");
        auto evtName = p.identifier();
        evtName->eval([](auto& m, IT b, IT e) {
            return m.str();
            });
        auto evt = p.optional(p.all("@", evtName));
        r_group = p.all("(", r_any, ")");
        r_item->add(r_regex, r_text, r_id, r_group, r_cut);
        r_option = p.all(r_item, "?");
        r_many = p.all(r_item, "*");
        r_many1 = p.all(r_item, "+");
        auto r_event = p.all(r_item, evt);

        r_till = p.all("...", r_item);
        r_list = p.all("[", r_item, r_item, "]");
        r_expr->add(r_till, r_many, r_many1, r_option, r_list, r_event, r_item);
        r_rule = p.all(r_id, "=", r_any, ";");
        r_ruleList = p.all(p.many1(r_rule), p.eof());

        r_id->eval([&](Match& m, IT b, IT e) {
            return (DSLNode*)create<DSLID>(getRange(p, m), m.str());
            });

        r_cut->eval([&](Match& m, IT b, IT e) {
            return (DSLNode*)create<DSLCUT>(getRange(p, m));
            });

        r_regex->eval([&](Match& m, IT b, IT e) {
            std::string s = m.str();
            s = s.substr(1, s.length() - 2);
            return (DSLNode*)create<DSLRegex>(getRange(p, m), s);
            });

        r_text->eval([&](Match& m, IT b, IT e) {
            auto s = m.str();
            std::string r;
            int rsz = 0;
            auto ret = parseCSTR(s.data(), s.length(), r, rsz);
            return (DSLNode*)create<DSLText>(getRange(p, m), r);
            });

        r_any->eval([&](Match& m, IT b, IT e) {
            auto* node = create<DSLAny>(getRange(p, m));
            while (b != e) {
                node->nodes.push_back(*(*b++).template get<DSLNode*>());
            }
            if (node->nodes.size() == 1) {
                DSLNode* n = node->nodes[0];
                return n;
            }
            return (DSLNode*)node;
            });

        r_event->eval([&](Match& m, IT b, IT e) {
            auto* node = *((b++)->get<DSLNode*>());
            if (b != e) {
                auto* name = b->get<std::string>();
                if (name) {
                    idMap.emplace(std::make_pair(*name, node));
                }
            }
            return node;
            }
        );
        r_all->eval([&](Match& m, IT b, IT e) {
            auto* node = create<DSLAll>(getRange(p, m));
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
            auto* node = create<DSLMany>(getRange(p, m), *(*b).template get<DSLNode*>());
            return (DSLNode*)node;
            });
        r_many1->eval([&](Match& m, IT b, IT e) {
            auto* node = create<DSLMany1>(getRange(p, m), *(*b).template get<DSLNode*>());
            return (DSLNode*)node;
            });
        r_list->eval([&](Match& m, IT b, IT e) {
            auto* item = *(*b++).template get<DSLNode*>();
            auto* dem = *(*b++).template get<DSLNode*>();
            auto* node = create<DSLList>(getRange(p, m), item, dem);
            return (DSLNode*)node;
            });
        r_till->eval([&](Match& m, IT b, IT e) {
            auto* item = *(*b++).template get<DSLNode*>();
            auto* node = create<DSLTill>(getRange(p, m), item);
            return (DSLNode*)node;
            });
        r_option->eval([&](Match& m, IT b, IT e) {
            auto* node = create<DSLOption>(getRange(p, m), *(*b).template get<DSLNode*>());
            return (DSLNode*)node;
            });
        r_rule->eval([&](Match& m, IT b, IT e) {
            DSLID* id = (DSLID*)*(*b++).template get<DSLNode*>();
            DSLRule* r = create<DSLRule>(getRange(p, m), *(*b++).template get<DSLNode*>());
            r->name = id->name;
            r->id = id;
            r->ruleLine = m.str();
            return (DSLNode*)r;
            });
        r_ruleList->eval([&](Match& m, IT b, IT e) {
            auto* node = create<DSLRuleList>(getRange(p, m));
            while (b != e) {
                node->nodes.push_back((DSLRule*)*(*b++).template get<DSLNode*>());
            }
            return (DSLNode*)node;
            });
    }

    KShared<KError> DSLContext::getLastError() {
        auto err = m_ruleParser.getLastError();
        if (err) {
            return err.clone();
        }
        if (checkRuleError) {
            return checkRuleError.clone();
        }
        auto err1 = m_userParser.getLastError();
        if (err1) {
            SyntaxError* serr = dynamic_cast<SyntaxError*>(err1.get());
            if (serr) {
                for(Rule*& r : serr->rule){
                    auto& info = r->getInfo();
                    if (!info.empty()) {
                        DSLNode** pnode = info.get<DSLNode*>();
                        if (pnode) {
                            auto* node = *pnode;
                            auto r = node->range->range();
                            auto locL = node->range->getRawSource()->getLocation(r.first);
                            auto locR = node->range->getRawSource()->getLocation(r.second);
                            auto posL = locL->location();
                            auto posR = locR->location();
                            std::cerr << "expect token at(" << posL.row << ":" << posL.col << ")" << std::endl;
                            std::cerr << posL.row << " | " << locL->getLine()->str() << std::endl;
                            std::cerr << "    ";
                            int i = 0;
                            for (i = 0; i < posL.col; ++i) {
                                std::cerr << " ";
                            }
                            std::cerr << "^";
                            i++;
                            for (; i < posR.col; ++i) {
                                std::cerr << " ";
                            }
                            std::cerr << "^";
                            std::cerr << std::endl;
                        }
                    }
                }
                
            }
            return err1;
        }
        return nullptr;
    }

    void DSLContext::prepareRules(const std::string& strRuleList) {
        m_strRule = strRuleList;
    }

    void DSLContext::prepareSkippedRule(CustomT&& p) {
        m_userParser.setSkippedRule(std::move(p));
    }

    void DSLContext::prepareConstant(const std::string& idName, CustomT&& p) {
        constantId.emplace(idName, p);
        auto* id = create<DSLID>(nullptr, idName, true);
        idMap.emplace(std::make_pair(idName, id));
    }

    DSLID* DSLContext::createId(KUnique<IRange> range, std::string& name) {
        auto cloneId = this->create<DSLID>(range, name, true);
        CustomT& f = constantId.at(name);
        cloneId->rule = m_userParser.custom(f);
        auto it = handleMap.find(name);
        if (it != handleMap.end()) {
            CaptureT f = it->second;
            cloneId->rule->eval(std::move(f));
        }
        return cloneId;
    }

    bool DSLContext::build() {
        auto m = r_ruleList->parse(m_strRule);
        if (!m) {
            auto err = m_ruleParser.getLastError();
            std::cerr << err << std::endl;
            return false;
        }
        DSLNode** d = m->capture<DSLNode*>(0);
        DSLRuleList* rlist = (DSLRuleList*)*d;

        bool succ = true;

        // pass, collect rule nodes
        auto hCollectId = [&](bool sink, DSLNode* n) {
            if (sink) {
                DSLRule* rule = dynamic_cast<DSLRule*>(n);
                if (rule) {
                    // unique
                    auto it = idMap.find(rule->name);
                    if (it != idMap.end()) {
                        checkRuleError = KShared<KError>(new RedefinedIDError(this->m_ruleParser.getSource(), rule->id));
                        succ = false;
                        return;
                    }
                    idMap.insert(it, std::make_pair(rule->name, rule->node));
                }
            }
        };
        rlist->visit(hCollectId);
        if (!succ) {
            return false;
        }

        // pass, replace id in idMap to final defination
        std::vector<std::pair<std::string, DSLNode*>> changes;
        {

            bool change = false;

            while (true) {
                change = false;
                for (auto& kv : idMap) {
                    auto& name = kv.first;
                    auto* node = kv.second;
                    auto* id = dynamic_cast<DSLID*>(node);
                    if (id == nullptr) {
                        continue;
                    }
                    if (id->isPreset) {
                        continue;
                    }
                    if (id->name == name) {
                        checkRuleError = KShared<KError>(new RecursiveIDError(this->m_ruleParser.getSource(), id));
                        succ = false;
                        break;
                    }
                    auto it = idMap.find(id->name);
                    if (it == idMap.end()) {
                        std::cerr << id->name << " is not defined " << std::endl;
                        checkRuleError = KShared<KError>(new UndefinedIDError(this->m_ruleParser.getSource(), id));
                        succ = false;
                        break;
                    }
                    kv.second = it->second;
                    change = true;
                }
                if (!succ || !change) {
                    break;
                }
            }
        }
        if (!succ) {
            return false;
        }

        // pass, check ids
        auto hCheckId = [&](bool sink, DSLNode* n) {
            if (sink) {
                DSLID* id = dynamic_cast<DSLID*>(n);
                if (id) {
                    auto it = idMap.find(id->name);
                    if (it == idMap.end()) {
                        checkRuleError = KShared<KError>(new UndefinedIDError(this->m_ruleParser.getSource(), id));
                        succ = false;
                        return;
                    }
                }
            }
        };
        rlist->visit(hCheckId);
        if (!succ) {
            return false;
        }


        // pass, replace id
        auto hReplaceId = [&](bool sink, DSLNode* n) {
            if (sink) {
                DSLWrap* wrap = dynamic_cast<DSLWrap*>(n);
                if (wrap) {
                    auto* id = dynamic_cast<DSLID*>(wrap->node);
                    if (!id) return;
                    auto it = idMap.find(id->name);
                    auto* stillId = dynamic_cast<DSLID*>(it->second);
                    if (stillId) {
                        // must be preset and is preset for being checked above
                        //if (stillId->isPreset) {
                       wrap->node = createId(id->range, stillId->name);
                        //}
                    }
                    else {
                        wrap->node = it->second;
                    }
                }
                DSLList* plist = dynamic_cast<DSLList*>(n);
                if (plist) {
                    auto* id = dynamic_cast<DSLID*>(plist->node);
                    if (id) {
                        auto it = idMap.find(id->name);
                        auto* stillId = dynamic_cast<DSLID*>(it->second);
                        if (stillId) {
                            // must be preset for being checked before
                            //if (stillId->isPreset) {
                            plist->node = createId(id->range, stillId->name);
                            //}
                        }
                        else {
                            plist->node = it->second;
                        }
                    }
                    id = dynamic_cast<DSLID*>(plist->dem);
                    if (id) {
                        auto it = idMap.find(id->name);
                        auto* stillId = dynamic_cast<DSLID*>(it->second);
                        if (stillId) {
                            // must be preset for being checked before
                            //if (stillId->isPreset) {
                            plist->dem = createId(id->range, stillId->name);
                            //}
                        }
                        else {
                            plist->dem = it->second;
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
                        auto* stillId = dynamic_cast<DSLID*>(it->second);
                        if (stillId) {
                            // must be preset for being checked before
                            //if (stillId->isPreset) {
                            children[i] = createId(id->range, stillId->name);
                            //}
                        }
                        else {
                            children[i] = it->second;
                        }
                    }
                }
            };
        };
        rlist->visit(hReplaceId);
        if (!succ) {
            return false;
        }

        // pass, replace id in idMap, in case of, a@abc
        for (auto& kv : idMap) {
            auto* id = dynamic_cast<DSLID*>(kv.second);
            if (id) {
                auto it = idMap.find(id->name);
                if (it != idMap.end()) {
                    kv.second = it->second;
                }
            }
        }

        // pass , check left recursive

        // pass , build RULES
        auto hBuild = [&](bool sink, DSLNode* n) {
            if (n->haveBuilt) {
                return;
            }
            if (!sink) {
                n->prepare(m_userParser);
                return;
            }
            else {
                auto r = n->build(m_userParser);
                n->haveBuilt = true;
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

        // pass, set rule info
        auto hSetInfo = [&](bool sink, DSLNode* n) {
            if (n->range) {
                if (n->rule) {
                    n->rule->setInfo(n);
                }
            }
        };
        rlist->visit(hSetInfo);
        if (!succ) {
            return false;
        }

        // bind eval
        for (auto& p : idMap) {
            auto name = p.first;
            auto it = handleMap.find(name);
            if (it != handleMap.end()) {
                auto handle = it->second;
                auto* node = p.second;
                if (node->rule) {
                    node->rule->eval([=](KLib42::Match& m, KLib42::IT arg, KLib42::IT noarg)->KAny {
                        return handle(m, arg, noarg);
                        });
                }
            }
        }

        return true;
    }
}