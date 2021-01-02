#include "impl.h"

namespace KParser {
    ParserImpl::ParserImpl(Parser* parser, size_t lookback, bool skipBlank) 
        :m_interface(parser), m_skipBlank(skipBlank), m_lookback(lookback), m_headMax(0) {
    }

    ParserImpl::~ParserImpl() {
        for (auto r : rules) {
            delete r;
        }
        reset();
    }

    void ParserImpl::reset() {
        m_toparse = "";
        m_headMax = 0;
    }
}