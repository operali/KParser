// author: operali
// desc: Domain special language of ENBF

#pragma once

#include <vector>
#include <unordered_map>
#include "./impl.h"
#include "./doc.h"
namespace KLib42 {

    struct DSLNode {
        KUnique<IRange> range;
        DSLNode(KUnique<IRange> range);

        virtual ~DSLNode() {}
        bool visiting = false;
        virtual void visit(std::function<void(bool sink, DSLNode* node)> handle);
        Rule* rule = nullptr;
        bool haveBuilt = false;
        virtual void prepare(Parser& p) {}
        virtual bool build(Parser& p) { return true; }
    };

    // only preset after build, others will be replacing with its rule node defination
    struct DSLID : public DSLNode {
        std::string name;
        bool isPreset;
        DSLID(KUnique<IRange> range, std::string name, bool isPreset = false) :DSLNode(range), name(name), isPreset(isPreset) {
        };
    };

    struct DSLCUT : public DSLNode {
        DSLCUT(KUnique<IRange> range) :DSLNode(range) {
        };
        bool build(Parser& p) override;
    };

    struct DSLText : public DSLNode {
        std::string name;
        DSLText(KUnique<IRange> range, std::string name) :DSLNode(range), name(name) {};
        void prepare(Parser& p) override;
    };


    struct DSLRegex : public DSLNode {
        std::string name;
        DSLRegex(KUnique<IRange> range, std::string name) :DSLNode(range), name(name) {};
        void prepare(Parser& p) override;
    };

    struct DSLWrap : public DSLNode {
        DSLNode* node;
        DSLWrap(KUnique<IRange> range, DSLNode* node) :DSLNode(range), node(node) {};
        void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
    };

    struct DSLMany : public DSLWrap {
        DSLMany(KUnique<IRange> range, DSLNode* node) :DSLWrap(range, node) {};

        bool build(Parser& p) override;
    };

    struct DSLMany1 : public DSLWrap {
        DSLMany1(KUnique<IRange> range, DSLNode* node) :DSLWrap(range, node) {};

        bool build(Parser& p) override;
    };

    struct DSLList : public DSLNode {
        DSLNode* node;
        DSLNode* dem;
        DSLList(KUnique<IRange> range, DSLNode* node, DSLNode* dem) :DSLNode(range), node(node), dem(dem) {};

        void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
        bool build(Parser& p) override;
    };

    struct DSLTill : public DSLWrap {
        DSLTill(KUnique<IRange> range, DSLNode* node) :DSLWrap(range, node) {};
        bool build(Parser& p) override;
    };

    struct DSLOption : public DSLWrap {
        DSLOption(KUnique<IRange> range, DSLNode* node) :DSLWrap(range, node) {};
        bool build(Parser& p) override;
    };

    struct DSLChildren : public DSLNode {
        std::vector<DSLNode*> nodes;
        DSLChildren(KUnique<IRange> range) :DSLNode(range) {};

        void visit(std::function<void(bool sink, DSLNode* node)> handle) override;
    };

    struct DSLAny : public DSLChildren {
        DSLAny(KUnique<IRange> range) :DSLChildren(range) {
        };
        void prepare(Parser& p) override;
        bool build(Parser& p) override;
    };

    struct DSLAll : public DSLChildren {
        DSLAll(KUnique<IRange> range) :DSLChildren(range) {};
        void prepare(Parser& p) override;
        bool build(Parser& p) override;
    };

    struct DSLRule : public DSLWrap {
        std::string name;
        DSLID* id;
        std::string ruleLine;
        DSLRule(KUnique<IRange> range, DSLNode* node) :DSLWrap(range, node) {
        }
        bool build(Parser& p) override;
    };

    struct DSLRuleList : public DSLChildren {
        DSLRuleList(KUnique<IRange> range) :DSLChildren(range) {};
    };

    struct DSLContext {
        // TODO, private
        Parser m_ruleParser;
        Parser m_userParser;
        std::vector<DSLNode*> m_nodes;
        
    private:
        std::string m_strRule;
        
        std::unordered_map <std::string, CustomT> constantId;
        std::unordered_map <std::string, DSLNode*> idMap;
        std::unordered_map <std::string, CaptureT> handleMap;

        // debug
        KShared<KError> checkRuleError;
        KDocument m_doc;
        // end debug
    public:
        
        Rule* r_id;
        Rule* r_cut;
        Rule* r_text; // `id`
        Rule* r_regex; // r`id`
        Rule* r_item; //  id, tex, re
        Rule* r_group; //  `(` r_any `)`;
        Rule* r_option; // item?
        Rule* r_many; // item*
        Rule* r_many1; // item+
        Rule* r_till; // ...item
        Rule* r_list; // [k, x]
        Rule* r_rule; // `id` `=` any `;`
        Rule* r_any; // seq(p.all , "|");
        Rule* r_all; // p.many1(item);
        Rule* r_ruleList; // 

        DSLContext();
        ~DSLContext();

        template<typename T, typename ...TS>
        T* create(TS... args) {
            T* node = new T(args...);
            m_nodes.push_back(node);
            return node;
        }
        DSLID* createId(KUnique<IRange> range, std::string& name);
        KShared<KError> getLastError();
        void prepareRules(const std::string& str);
        void prepareSkippedRule(CustomT&& p);
        void prepareConstant(const std::string& idName, CustomT&& p);
        void prepareCapture(const std::string& evtName, CaptureT&& handle);
        bool build();
        KUnique<Match> parse(const std::string& ruleName, const std::string& str);
    };

    template<>
    inline std::string to_string(DSLNode*& v) {
        std::stringstream ss;
        ss << "dsl_node(";
        ss << reinterpret_cast<size_t>(v);
        ss << ")";
        return ss.str();
    }
}  