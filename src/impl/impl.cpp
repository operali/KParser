#include "impl.h"

namespace KLib42 {
    ParserImpl::ParserImpl(Parser* parser, KUSIZE lookback, bool skipBlank)
        :m_interface(parser), m_skipBlank(skipBlank), m_lookback(lookback), m_headMax(0), m_cache(nullptr), length(0) {
    }

    ParserImpl::~ParserImpl() {
        for (auto r : rules) {
            delete r;
        }
        reset();
    }

    void ParserImpl::reset() {
        m_text.setText("");
        m_cache = "";
        length = 0;
        m_headMax = 0;
        
        m_expStk.resize(64);
        m_expStk.clear();
    }


    void ParserImpl::setText(const std::string& text) {
        m_text.setText(text);
        auto s = m_text.getSource();
        m_cache = s->buff();
        length = s->len();
    }

    void ParserImpl::genParseError() {
        const char* errLoc = m_cache + m_headMax;
        parseErrInfo.reset(new SyntaxError(m_interface->getSource(), m_headMax ));
    }
}