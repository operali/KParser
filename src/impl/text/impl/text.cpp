#include <vector>
#include <string>
#include <algorithm>
#include "../text.h"

namespace KLib42 {

	struct KTextImpl {
		char* _buff;
		int _len = -1;

		std::vector<int> m_ll;

		void setText(const std::string& text) {
			reset();
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

		inline void reset() {
			if (_len != -1) {
				delete[] _buff;
				_buff = nullptr;
				_len = -1;
			}
			m_ll.clear();
			m_ll.reserve(64);
		}

		inline ~KTextImpl() {
			reset();
		}
	};

	KText::KText() {
		pImpl = new KTextImpl();
	}

	void KText::setText(const std::string& text) {
		pImpl->setText(text);
	};

	struct KSource : public ISource {
		KTextImpl* impl;
		KSource(KTextImpl* impl) :impl(impl) {};
		~KSource() override = default;
		const char* buff() override {
			return impl->_buff;
		};

		size_t len() override {
			return impl->_len;
		};
		KShared<IEnumerator<ILine>> lines() override;
		size_t lineCount() override;
		
		KShared<ILine> getLine(size_t index) override;
		KShared<ILocation> getLocation(size_t index) override;
	};

	struct KLine : public ILine {
		KTextImpl* impl;
		size_t _idx;
		size_t index() override {
			return _idx;
		}
		std::string str() override;
		KShared<IRange> toRange() override;
		KLine(KTextImpl* impl, size_t idx): impl(impl), _idx(idx){}
		~KLine() override= default;
	};

	struct KLocation : public ILocation {
		KTextImpl* impl;
		size_t _idx;
		KLocation(KTextImpl* impl, size_t _idx):impl(impl),_idx(_idx) {}
		~KLocation() override = default;
		size_t index() override {
			return _idx;
		}
		loc location() override {
			auto m_ll = impl->m_ll;
			auto it = std::lower_bound(m_ll.begin(), m_ll.end(), _idx);
			auto row = it - m_ll.begin();
			auto left = (it == m_ll.begin()) ? 0 : *(it - 1) + 1;
			auto right = (it == m_ll.end()) ? impl->_len : *it;
			auto col = _idx - (size_t)left;
			return loc{ (size_t)row, col, (size_t)left, (size_t)right, _idx };
		}

		KShared<ILine> getLine() override {
			auto m_ll = impl->m_ll;
			auto it = std::lower_bound(m_ll.begin(), m_ll.end(), _idx);
			auto row = it - m_ll.begin();
			return KShared<ILine>(new KLine{ impl, (size_t)row });
		};
		
		KShared<IRange> getRange() override;
	};

	KShared<ISource> KText::getSource() {
		return KShared<ISource>(new KSource(this->pImpl));
	}

	struct Lines : public IEnumerator<ILine> {
		KTextImpl* impl;
		size_t index;
		Lines(KTextImpl* impl, size_t index) :impl(impl), index(0) {};
		inline ~Lines() override {};
		bool hasNext() override {
			return index != impl->m_ll.size();
		}

		KShared<ILine> next() override {
			if (index == impl->m_ll.size()) {
				throw std::exception();
			}
			return KShared<ILine>(new KLine{ impl, index++ });
		}

		std::vector<KShared<ILine>> toArray() override {
			std::vector<KShared<ILine>> ret;
			while (hasNext()) {
				ret.emplace_back(next());
			}
			return std::move(ret);
		}
	};

	struct KRange : public IRange {
		KTextImpl* impl;
		size_t left = 0;
		size_t right = 0;

		KRange(KTextImpl* impl, size_t left, size_t right):impl(impl), left(left), right(right) {
		}

		~KRange() = default;

		size_t from() override;
		size_t to() override;

		std::string str() override;
	};

	KShared<IEnumerator<ILine>> KSource::lines() {
		return KShared<IEnumerator<ILine>>(new Lines{ impl, 0 });
	}

	size_t KSource::lineCount() {
		return this->impl->m_ll.size();
	}
	
	KShared<ILine> KSource::getLine(size_t index) {
		if (index >= this->impl->m_ll.size()) {
			return KShared<ILine>();
		}
		return KShared<ILine>(new KLine{ impl, (size_t)index });
	}

	KShared<ILocation> KSource::getLocation(size_t index) {
		if (index >= this->impl->_len) {
			throw std::exception();
		}
		return KShared<ILocation>(new KLocation{ impl, (size_t)index });
	}

	KShared<IRange> KLine::toRange() {
		auto m_ll = impl->m_ll;
		auto locIdx = m_ll.at(_idx);
		if (_idx == 0) {
			return KShared<IRange>(new KRange{ impl, 0, (size_t)locIdx });
		}
		else {
			auto left = m_ll.at(_idx-1);
			return KShared<IRange>(new KRange{ impl, (size_t)left+1, (size_t)locIdx });
		}
	};

	size_t KRange::from() {
		return left;
	};

	size_t KRange::to() {
		return right;
	};

	std::string KRange::str() {
		return std::string(left, right);
	}

	std::string KLine::str() {
		auto r = toRange();
		auto* c = impl->_buff;
		auto left = c + r->from();
		auto right = c + r->to();
		return std::string(left, right);
	}

	KShared<IRange> KLocation::getRange() {
		return KShared<IRange>(new KRange{ impl, _idx, _idx });
	}
}


