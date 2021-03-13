// author: operali
// desc: Domain special language of ENBF

#include <iostream>
#include "common.h"
#include "../kparser.h"
#include "./dsl.h"
#include "./util.h"
#include "./impl.h"

namespace KLib42 {

    DSLNode::DSLNode(KUnique<IRange> range) :range(range){
    };

    void DSLNode::visit(std::function<void(bool sink, DSLNode * node)> handle) {
        handle(false, this);
        handle(true, this);
    }

    void DSLID::prepare(KLib42::Parser& p) {
        if (name == "ID") {
            rule = p.identifier();
            rule->eval([&](Match& m, IT b, IT e) {
                return m.str();
                });
        }
        else if (name == "NUM") {
            rule = p.float_();
            rule->eval([&](Match& m, IT b, IT e) {
                auto s = m.str();
                double ret;
                int len;
                parseFloat(s.c_str(), s.length(), ret, len);
                return ret;
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

        idMap.emplace(std::make_pair("ID", create<DSLID>(nullptr, "ID" , true )));
        idMap.emplace(std::make_pair("NUM", create<DSLID>(nullptr, "NUM", true )));
        idMap.emplace(std::make_pair("NONE", create<DSLID>(nullptr, "NONE", true)));
        idMap.emplace(std::make_pair("EOF", create<DSLID>(nullptr, "EOF", true)));

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
            if(r){
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
        r_item->add(r_regex, r_text, r_id, r_group);
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
        r_regex->eval([&](Match& m, IT b, IT e) {
            std::string s = m.str();
            s = s.substr(1, s.length() - 2);
            return (DSLNode*)create<DSLRegex>(getRange(p, m), s );
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
            auto* node = *((b++)-> get<DSLNode*>());
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
                auto infoId = serr->rule->getInfo();
                if (infoId != -1) {
                    auto loc = m_ruleParser.getSource()->getLocation(infoId);
                    std::cerr << loc->getLine()->str();
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
        auto* id = create<DSLID>(nullptr, idName , true);
        id->rule = m_userParser.custom(p);
        idMap.emplace(std::make_pair(idName, id));
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

        // pass, collect p.all ID
        auto hCheckID = [&](bool sink, DSLNode* n) {
            if (sink) {
                if (&typeid(*n) == &typeid(DSLRule)) {
                    DSLRule* rule = (DSLRule*)n;
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
                    checkRuleError = KShared<KError>(new UndefinedIDError(this->m_ruleParser.getSource(), id));
                    succ = false;
                    break;
                }
                node = it->second;
                if (node == kv.second) {
                    checkRuleError = KShared<KError>(new RecursiveIDError(this->m_ruleParser.getSource(), id));
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
                            /*auto* r = new DSLAny{ this, id->range };
                            r->nodes.push_back(rn);
                            rule->node = r;
                            idMap[rule->name] = r;*/
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
                                checkRuleError = KShared<KError>(new UndefinedIDError(this->m_ruleParser.getSource(), id));
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
                        checkRuleError = KShared<KError>(new UndefinedIDError(this->m_ruleParser.getSource(), id));
                        // lastError = std::string("invalid id of ") + child->name;
                        return;
                    }
                }
                DSLID* idNode = dynamic_cast<DSLID*>(n);
                if (idNode) {
                    auto it = idMap.find(idNode->name);
                    if (it == idMap.end()) {
                        succ = false;
                        checkRuleError = KShared<KError>(new UndefinedIDError(this->m_ruleParser.getSource(), idNode));
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
                            checkRuleError = KShared<KError>(new UndefinedIDError(this->m_ruleParser.getSource(), id));
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
                            checkRuleError = KShared<KError>(new UndefinedIDError(this->m_ruleParser.getSource(), id));
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
                            checkRuleError = KShared<KError>(new UndefinedIDError(this->m_ruleParser.getSource(), id));
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
                    n->prepare(m_userParser);
                    return;
                }
                else {
                    auto r = n->build(m_userParser);
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

            // pass, set rule info
            auto hSetInfo = [&](bool sink, DSLNode* n) {
                if (n->range) {
                    auto rgn = n->range->range();
                    if (n->rule) {
                        n->rule->setInfo(rgn.first);
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
                    node->rule->eval([=](KLib42::Match& m, KLib42::IT arg, KLib42::IT noarg)->KAny {
                        return handle(m, arg, noarg);
                        });
                }
            }

            return true;
        }
    }