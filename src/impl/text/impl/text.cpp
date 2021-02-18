#include <vector>
#include <string>
#include <algorithm>
#include "../text.h"

namespace KLib42 {
	struct KSourceImpl {
		char* _buff;
		int _len = -1;

		std::vector<int> m_ll;

		void setText(const std::string& text) {
			release();
			_len = text.length();

			_buff = new char[_len + 1];
			const char* src = text.data();
			char* des = _buff;
			const char* end = src + _len;
			for (; src != end; ++src, ++des) {
				auto ch = *des = *src;
				if (ch == '\n') {
					m_ll.push_back(des-_buff);
				}
			}
			m_ll.push_back(des - _buff);
			*des = '\0';
		}

		inline void release() {
			if (_len != -1) {
				delete[] _buff;
				_buff = nullptr;
				_len = -1;
			}
			m_ll.clear();
			m_ll.reserve(64);
		}

		inline ~KSourceImpl() {
			delete[] _buff;
		}

		KSourceImpl() {
			setText("");
		}
	};

	KSource::KSource() {
		pImpl = new KSourceImpl();
	}

	KSource::~KSource() {
		delete pImpl;
	}

	void KSource::setText(const std::string& text) {
		pImpl->setText(text);
	}

	const char* KSource::raw() {
		return pImpl->_buff;
	}

	size_t KSource::len() {
		return pImpl->_len;
	}

	size_t KSource::lineCount() {
		return pImpl->m_ll.size();
	}

	struct KLine : public ILine {
		KSource* impl;
		ISource* getRawSource() override {
			return impl;
		}
		size_t _idx;
		size_t index() override {
			return _idx;
		}
		std::string str() override;
		KUnique<ILine> previous() override;
		KUnique<ILine> next() override;
		KUnique<IRange> toRange() override;
		KLine(KSource* impl, size_t idx): impl(impl), _idx(idx){}
		~KLine() override= default;
	};

	struct KLocation : public ILocation {
		KSource* impl;
		ISource* getRawSource() override {
			return impl;
		}
		size_t _idx;
		KLocation(KSource* impl, size_t _idx):impl(impl),_idx(_idx) {}
		~KLocation() override = default;
		size_t index() override {
			return _idx;
		}
		loc location() override {
			auto m_ll = impl->pImpl->m_ll;
			auto it = std::lower_bound(m_ll.begin(), m_ll.end(), _idx);
			auto row = it - m_ll.begin();
			auto left = (it == m_ll.begin()) ? 0 : *(it - 1) + 1;
			auto right = (it == m_ll.end()) ? impl->pImpl->_len : *it;
			auto col = _idx - (size_t)left;
			return loc{ (size_t)row, col, (size_t)left, (size_t)right, _idx };
		}

		KUnique<ILine> getLine() override {
			auto m_ll = impl->pImpl->m_ll;
			auto it = std::lower_bound(m_ll.begin(), m_ll.end(), _idx);
			auto row = it - m_ll.begin();
			return KUnique<ILine>(new KLine{ impl, (size_t)row });
		};
		
		KUnique<IRange> getRange() override;
	};

	struct KRange : public IRange {
		KSource* impl;
		ISource* getRawSource() override {
			return impl;
		}
		size_t left = 0;
		size_t right = 0;

		KRange(KSource* impl, size_t left, size_t right):impl(impl), left(left), right(right) {
		}
		
		std::pair<size_t, size_t> range() override {
			return std::make_pair(left, right);
		}

		~KRange() = default;

		std::string str() override;
	};

	KUnique<ILine> KSource::getLine(size_t index) {
		if (index >= this->pImpl->m_ll.size()) {
			return KUnique<ILine>();
		}
		return KUnique<ILine>(new KLine{ this, (size_t)index });
	}

	KUnique<ILocation> KSource::getLocation(size_t index) {
		if (index > this->pImpl->_len) {
			return KUnique<ILocation>();
		}
		return KUnique<ILocation>(new KLocation{ this, (size_t)index });
	}

	KUnique<IRange> KSource::getRange(size_t from, size_t to) {
		if (from >= this->pImpl->_len || to > this->pImpl->_len || from > to) {
			return KUnique<IRange>();
		}
		return KUnique<IRange>(new KRange(this, from, to));
	}

	KUnique<IRange> KLine::toRange() {
		auto m_ll = impl->pImpl->m_ll;
		auto locIdx = m_ll.at(_idx);
		if (_idx == 0) {
			return KUnique<IRange>(new KRange{ impl, 0, (size_t)locIdx });
		}
		else {
			auto left = m_ll.at(_idx-1);
			return KUnique<IRange>(new KRange{ impl, (size_t)left+1, (size_t)locIdx });
		}
	};

	std::string KRange::str() {
		auto l = impl->pImpl->_buff + left;
		auto r = impl->pImpl->_buff + right;
		return std::string(l, r);
	}

	std::string KLine::str() {
		auto ir = toRange();
		return ir->str();
	}

	KUnique<ILine> KLine::previous() {
		if (_idx == 0) {
			return KUnique<ILine>();
		}
		else {
			return KUnique<ILine>(new KLine(impl, _idx - 1));
			
		}
	}

	KUnique<ILine> KLine::next() {
		if (_idx+1 == impl->pImpl->m_ll.size()) {
			return KUnique<ILine>();
		}
		else {
			return KUnique<ILine>(new KLine(impl, _idx + 1));
		}
	}

	KUnique<IRange> KLocation::getRange() {
		return KUnique<IRange>(new KRange{impl, _idx, _idx });
	}
}


