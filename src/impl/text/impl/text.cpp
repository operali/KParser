#include <vector>
#include <string>
#include <algorithm>
#include "../text.h"

namespace KLib42 {

	struct KTextImpl {
		char* _buff;
		KSIZE _len = -1;

		std::vector<KSIZE> m_ll;

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

		KUSIZE len() override {
			return impl->_len;
		};
		KShared<IEnumerator<ILine>> lines() override;
		KUSIZE lineCount() override;
		
		KShared<ILine> getLine(size_t index) override;
		KShared<ILocation> getLocation(size_t index) override;
	};

	struct KLine : public ILine {
		KTextImpl* impl;
		KUSIZE _idx;
		KUSIZE index() override {
			return _idx;
		}
		std::string str() override;
		KShared<IRange> toRange() override;
		KLine(KTextImpl* impl, KUSIZE idx): impl(impl), _idx(idx){}
		~KLine() override= default;
	};

	struct KLocation : public ILocation {
		KTextImpl* impl;
		KUSIZE _idx;
		KLocation(KTextImpl* impl, KUSIZE _idx):impl(impl),_idx(_idx) {}
		~KLocation() override = default;
		KUSIZE index() override {
			return _idx;
		}
		loc location() override {
			auto m_ll = impl->m_ll;
			auto it = std::lower_bound(m_ll.begin(), m_ll.end(), _idx);
			auto row = it - m_ll.begin();
			auto left = (it == m_ll.begin()) ? 0 : *(it - 1) + 1;
			auto col = _idx - (KUSIZE)left;
			return loc{ (KUSIZE)row, col };
		}

		KShared<ILine> getLine() override {
			auto m_ll = impl->m_ll;
			auto it = std::lower_bound(m_ll.begin(), m_ll.end(), _idx);
			auto row = it - m_ll.begin();
			return KShared<ILine>(new KLine{ impl, (KUSIZE)row });
		};
	};

	KShared<ISource> KText::getSource() {
		return KShared<ISource>(new KSource(this->pImpl));
	}

	struct Lines : public IEnumerator<ILine> {
		KTextImpl* impl;
		KUSIZE index;
		Lines(KTextImpl* impl, KUSIZE index) :impl(impl), index(0) {};
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
		KUSIZE left = 0;
		KUSIZE right = 0;

		KRange(KTextImpl* impl, KUSIZE left, KUSIZE right):impl(impl), left(left), right(right) {
		}

		~KRange() = default;

		size_t from() override;
		size_t to() override;

		std::string KRange::str() override;
	};

	KShared<IEnumerator<ILine>> KSource::lines() {
		return KShared<IEnumerator<ILine>>(new Lines{ impl, 0 });
	}

	KUSIZE KSource::lineCount() {
		return this->impl->m_ll.size();
	}
	
	KShared<ILine> KSource::getLine(size_t index) {
		if (index >= this->impl->m_ll.size()) {
			throw std::exception();
		}
		return KShared<ILine>(new KLine{ impl, (KUSIZE)index });
	}

	KShared<ILocation> KSource::getLocation(size_t index) {
		if (index >= this->impl->_len) {
			throw std::exception();
		}
		return KShared<ILocation>(new KLocation{ impl, (KUSIZE)index });
	}

	KShared<IRange> KLine::toRange() {
		auto m_ll = impl->m_ll;
		auto locIdx = m_ll.at(_idx);
		if (_idx == 0) {
			return KShared<IRange>(new KRange{ impl, 0, (KUSIZE)locIdx });
		}
		else {
			auto left = m_ll.at(_idx-1);
			return KShared<IRange>(new KRange{ impl, (KUSIZE)left+1, (KUSIZE)locIdx });
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
}
