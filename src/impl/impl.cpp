#include "impl.h"

namespace KParser {
    ParserImpl::ParserImpl(Parser* parser, size_t lookback, bool skipBlank)
        :m_interface(parser), m_skipBlank(skipBlank), m_lookback(lookback), m_headMax(0), m_cache(nullptr), length(0) {
    }

    ParserImpl::~ParserImpl() {
        for (auto r : rules) {
            delete r;
        }
        reset();
    }

    void ParserImpl::reset() {
        if (length != 0) {
            delete[] m_cache;
        }
        m_headMax = 0;
        libany::any obj;
        m_expStk.resize(20);
        m_expStk.clear();
    }
}