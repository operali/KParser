#pragma once

#include "../KParser.h"
#include "common.h"
#include "rule.h"





namespace KParser {
    
    struct DataStackImpl : public DataStack {
        std::vector<std::any> m_datas;

        void push_any(std::any&& t) override;

        std::any pop_any() override;

        std::any& get_any(size_t i) override;

        void set_any(std::any&& t, size_t i) override;

        void clear() override;

        size_t size() override;
    };
    struct RuleNode;
    struct Parser;
    struct ParserImpl {
        Parser* m_interface;
        bool m_skipBlank;
        size_t m_lookback;
        size_t m_headMax;
        
        char* m_cache;
        size_t length;
        VecT<RuleNode*> rules;
        DataStackImpl dataStk;

        void setText(const std::string& text) {
            length = text.length();
            if (length == 0) {
                m_cache = nullptr;
            }
            else {
                m_cache = new char[length];
                memcpy(m_cache, &(*text.begin()), length);
            }
        }

        ParserImpl(Parser* parser, size_t lookback, bool skipBlank);
        ~ParserImpl();
        void reset();
    };





}
