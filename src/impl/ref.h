
#pragma once

namespace KParser {
    struct KCounter;
    template<typename T>
    struct KShared;

	template<typename T>
	struct KWeakShared;

	struct KCounter {
		size_t count = 0;
		size_t wcount = 0;
		inline void share() {
			count++;
			if (count == 1) {
				wcount++;
			}
		}

		inline void addWeak() {
			wcount++;
		}

		inline bool rmWeak() {
			wcount--;
			if (wcount == 0) {
				delete this;
				return false;
			}
			return true;
		}

		inline bool rmShare() {
			count--;
			if (count == 0) {
				return rmWeak();
			}
			return true;
		}

		inline bool exist() {
			return count > 0;
		}
	};

	template<typename T>
	struct KRawShared {
	protected:
		KCounter* _counter;
		T* _reference;
		KRawShared(KCounter* _counter, T* _reference) :_counter(_counter), _reference(_reference) {};
		KRawShared(KWeakShared<T>&) = delete;
		T* getRef();
	public:
		friend struct KShared<T>;
		friend struct KWeakShared<T>;
		inline T* operator->() {
			return getRef();
		}
		inline T& operator*() {
			return *getRef();
		}
		size_t refCount();
		size_t weakCount();
		bool exist();
	};

	// not shared
	template<typename T>
	struct KWeakShared : public KRawShared<T> {
	private:
		KWeakShared(KCounter* _counter, T* _reference) :KRawShared(_counter, _reference) {
			_counter->addWeak();
		};
		friend struct KShared<T>;
		void release();
		KWeakShared(T* obj) = delete;
	public:
		KShared<T> getShared();
		KWeakShared(KWeakShared<T>&& other);
		inline ~KWeakShared() {
			release();
		}
	};

	// shared
	template<typename T>
	struct KShared : public KRawShared<T> {
	private:
		void release();
		friend struct KWeakShared<T>;
		KShared(KShared& other);
	public:
		void share();
		KWeakShared<T> getWeak();
		KShared clone();
		KShared(KShared<T>&& other);
		KShared(T* _reference) :KRawShared(new KCounter(), _reference) {
			_counter->share();
		};
		inline ~KShared() {
			release();
		}
	};

	template<typename T>
	void KShared<T>::share() {
		if(_counter)_counter->share();
	}

	template<typename T>
	void KShared<T>::release() {
		if (_counter && !_counter->rmShare()) {
			delete _reference;
		}
	}

	template<typename T>
	T* KRawShared<T>::getRef() {
		return _reference;
	}

	template<typename T>
	KWeakShared<T> KShared<T>::getWeak() {
		KWeakShared<T> w(this->_counter, this->_reference);
		return std::move(w);
	}

	template<typename T>
	inline KShared<T> makeShared(T&& obj) {
		return KShared<T>(new T(std::move(obj)));
	}

	template<typename T>
	KShared<T> KShared<T>::clone() {
		KShared<T> r(*this);
		return std::move(r);
	}
	template<typename T>
	void KWeakShared<T>::release() {
		if(_counter)_counter->rmWeak();
	}

	template<typename T>
	KShared<T> KWeakShared<T>::getShared() {
		if (exist()) {
			KShared<T> s{ _counter, _reference };
			return std::move(s);
		}
		throw std::exception();
	}

	template<typename T>
	KWeakShared<T>::KWeakShared(KWeakShared&& other):KRawShared(other._counter, other._reference) {
		other._counter = nullptr;
	}

	template<typename T>
	KShared<T>::KShared(KShared&& other) : KRawShared{other._counter,  other._reference} {
		other._counter = nullptr;
		other._reference = nullptr;
	}

	template<typename T>
	bool KRawShared<T>::exist() {
		return _counter->count != 0;
	}

	template<typename T>
	size_t KRawShared<T>::refCount() {
		return _counter->count;
	}

	template<typename T>
	size_t KRawShared<T>::weakCount() {
		return _counter->wcount;
	}

	template<typename T>
	KShared<T>::KShared(KShared<T>& other):KRawShared<T>(other._counter, other._reference){
		_counter->share();
	}

	template <typename T>
	struct IEnumerator {
		virtual bool hasNext() = 0;
		virtual KShared<T> next() = 0;
		virtual std::vector<KShared<T>> toArray() = 0;
		virtual ~IEnumerator() = default;
	};
}