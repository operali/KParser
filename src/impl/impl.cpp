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
        dataStk.clear();
        std::any obj;
        obj.swap(m_value);
    }


    void DataStackImpl::push_any(std::any&& t) {
        m_datas.emplace_back(t);
    };

    std::any DataStackImpl::pop_any() {
        std::any r;
        swap(m_datas.back(), r);
        m_datas.pop_back();
        return std::move(r);
    }

    std::any& DataStackImpl::get_any(size_t i) {
        return m_datas.at(i);
    }

    void DataStackImpl::set_any(std::any&& t, size_t i) {
        std::swap(m_datas.at(i), t);
    }

    void DataStackImpl::clear() {
        m_datas.clear();
    }

    size_t DataStackImpl::size() {
        return m_datas.size();
    }
}