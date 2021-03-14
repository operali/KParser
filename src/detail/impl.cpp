#include "impl.h"

namespace KLib42 {
    ParserImpl::ParserImpl(Parser* parser, KUSIZE lookback, bool skipBlank, CustomT skipRule)
        :m_interface(parser), 
        m_skipBlank(skipBlank), 
        m_lookback(lookback), 
        m_headMax(0), 
        m_cache(nullptr), length(0), m_text(new KSource()), m_skipRule(skipRule), m_trace(false){
    }

    ParserImpl::~ParserImpl() {
        for (auto r : rules) {
            delete r;
        }
        reset();
    }

    void ParserImpl::reset() {
        m_text->setText("");
        m_cache = "";
        length = 0;
        m_headMax = 0;
        
        m_headRule.clear();

        m_expStk.resize(64);
        m_expStk.clear();

        m_ss.clear();
        m_parseErrInfo.reset(nullptr);

        /*for (auto& p : m_ruleInfoMap) {
            delete p.second;
        }
        m_ruleInfoMap.clear();*/
    }

    void ParserImpl::setText(const std::string& text) {
        m_text->setText(text);
        m_cache = m_text->raw();
        length = m_text->len();
    }

    void ParserImpl::genParseError() {
        const char* errLoc = m_cache + m_headMax;
        m_parseErrInfo.reset(new SyntaxError(m_interface->getSource(), m_headRule, m_headMax ));
    }
}