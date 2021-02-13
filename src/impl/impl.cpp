#include "impl.h"

namespace KLib42 {
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
        
        m_expStk.resize(64);
        m_expStk.clear();
        m_ll.resize(64);
        m_ll.clear();

        parseErrInfo.row = 0;
        parseErrInfo.lineMid = nullptr;
        parseErrInfo.lineLeft = nullptr;
        parseErrInfo.lineRight = nullptr;
    }


    void ParserImpl::setText(const std::string& text) {
        length = text.length();
        
        m_cache = new char[length + 1];
        const char* src = text.data();
        char* des = m_cache;
        const char* end = src + length;
        for (; src != end; ++src, ++des) {
            auto ch = *des = *src;
            if (ch == '\n') {
                m_ll.push_back(des);
            }
        }
        m_ll.push_back(des);
        *des = '\0';
    }

    void ParserImpl::genParseError() {
        char* errLoc = m_cache + m_headMax;
        auto it = std::lower_bound(m_ll.begin(), m_ll.end(), errLoc);
        parseErrInfo.row = it - m_ll.begin();
        parseErrInfo.lineMid = m_cache + m_headMax;
        parseErrInfo.lineLeft = (it == m_ll.begin()) ? m_cache : *(it - 1) + 1;
        parseErrInfo.col = parseErrInfo.lineMid - parseErrInfo.lineLeft;
        parseErrInfo.lineRight = *it;
    }
}