
#pragma once
#include<vector>
#include<assert.h>
namespace KLib42 {

	// unique_ptr for simple using
	template<typename T>
	struct KUnique
	{
		T* _data;
		KUnique(const KUnique&) = delete;
		KUnique& operator= (const KUnique& rhs) = delete;
	public:
		inline KUnique(T* data = nullptr)
			: _data(data) {};

		KUnique(KUnique&& rhs) noexcept :_data(rhs._data) {
			rhs._data = nullptr;
		}

		
		inline ~KUnique() {
			_destruct();
		}

		inline void reset(T* pData) {
			_destruct();
			_data = pData;
		}

		inline T* release() {
			T* pTemp = _data;
			_data = nullptr;
			return pTemp;
		}

		inline T* get()
		{
			return _data;
		}

		inline operator bool() const
		{
			return _data != nullptr;
		}

		inline T& operator * ()
		{
			return *_data;
		}

		inline T* operator -> ()
		{
			return _data;
		}
		
	private:
		inline void _destruct()
		{
			if (nullptr == _data) return;
			delete _data;
			_data = nullptr;
		}
	};

	template<typename T>
	inline KUnique<T> makeUnique(T&& obj) {
		return KUnique<T>(new T(std::move(obj)));
	}

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

		bool exist() const
		{
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
		T* get();
	public:
		friend struct KShared<T>;
		friend struct KWeakShared<T>;
		inline T* operator->() {
			return get();
		}
		inline T& operator*() {
			return *get();
		}
		size_t refCount();
		size_t weakCount();
		operator bool();
	};


	template<typename T>
	T* KRawShared<T>::get() {
		if (_counter && _counter->count != 0) {
			return _reference;
		}
		return nullptr;
	}

	template<typename T>
	KRawShared<T>::operator bool() {
		return _counter &&  _counter->exist();
	}

	template<typename T>
	size_t KRawShared<T>::refCount() {
		return _counter?_counter->count:0;
	}

	template<typename T>
	size_t KRawShared<T>::weakCount() {
		return _counter?_counter->wcount:0;
	}


	// not shared
	template<typename T>
	struct KWeakShared : public KRawShared<T> {
	private:
		KWeakShared(KCounter* _counter, T* _reference) :KRawShared<T>(_counter, _reference) {
			if(_counter)_counter->addWeak();
		};
		friend struct KShared<T>;
		KWeakShared(T* obj) = delete;
	public:
		KWeakShared(KWeakShared<T>&& other);
		inline ~KWeakShared() {
			if (this->_counter) {
				this->_counter->rmWeak();
			}
		}
	};

	template<typename T>
	KWeakShared<T>::KWeakShared(KWeakShared&& other) :KRawShared<T>(other._counter, other._reference) {
		other._counter = nullptr;
	}

	// shared
	template<typename T>
	struct KShared : public KRawShared<T> {
	private:
		friend struct KWeakShared<T>;
		void share();
	public:
		KWeakShared<T> getWeak();
		KShared clone();
		KShared(KShared<T>&& other) noexcept;
		KShared(KShared& other) noexcept;
		KShared(T* _reference = nullptr) noexcept :KRawShared<T>(nullptr, _reference) {
			if (_reference) {
				this->_counter = new KCounter();
				this->_counter->share();
			}
		};
		void release();
		void reset(T* reference);
		KShared<T>& operator=(const KShared<T>& rh);
		~KShared() {
			release();
		}
	};


	template<typename T>
	void KShared<T>::share() {
		if(this->_counter)this->_counter->share();
	}

	template<typename T>
	void KShared<T>::release() {
		if (this->_counter && this->_counter->count != 0) {
			if (!this->_counter->rmShare()) {
				delete this->_reference;
			}
			this->_counter = nullptr;
		}
	}

	template<typename T>
	void KShared<T>::reset(T* reference) {
		release();
		this->_reference = reference;
		if (reference) {
			this->_counter = new KCounter();
			this->_counter->share();
		}
	}

	template<typename T>
	KShared<T>& KShared<T>::operator=(const KShared<T>& rh) {
		this->release();
		this->_counter = rh._counter;
		this->_reference = rh._reference;
		if (this->_counter) {
			this->_counter->share();
		}
		return *this;
	}

	template<typename T>
	KWeakShared<T> KShared<T>::getWeak() {
		return KWeakShared<T>(this->_counter, this->_reference);
	}

	template<typename T>
	inline KShared<T> makeShared(T&& obj) {
		return KShared<T>(new T(std::move(obj)));
	}

	template<typename T>
	KShared<T> KShared<T>::clone() {
		return KShared<T>{*this};
	}


	template<typename T>
	KShared<T>::KShared(KShared&& other) noexcept: KRawShared<T>{other._counter,  other._reference} {
		other._counter = nullptr;
		other._reference = nullptr;
	}


	template<typename T>
	KShared<T>::KShared(KShared<T>& other) noexcept :KRawShared<T>(other._counter, other._reference){
		if (other._reference) {
			this->_counter->share();
		}
	}

	template <typename T>
	struct IEnumerator {
		virtual bool hasNext() = 0;
		virtual KShared<T> next() = 0;
		virtual std::vector<KShared<T>> toArray() = 0;
		virtual ~IEnumerator() = default;
	};
}