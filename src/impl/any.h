#pragma once
#include <algorithm>
#include <tuple>

namespace KLib42 {
    class KAny {
        /*template<typename T> struct TID { static void id(){} };
        template<typename T> static size_t TOF() { return reinterpret_cast<size_t>(TID<T>::id); }
        template<typename T> static size_t TOF(T& ob) { return reinterpret_cast<size_t>(TID<T>::id); }*/
        template<typename T> static size_t TOF() { return reinterpret_cast<size_t>(&typeid(T)); }
        // template<typename T> static size_t TOF(T& ob) { return reinterpret_cast<size_t>(TID<T>::id); }

        struct DataHolderBase {
            virtual ~DataHolderBase() {}
            virtual size_t id() = 0;
            virtual DataHolderBase* clone() = 0;
            template<typename T>
            inline size_t is() {
                return id() == TOF<T>();
            }
        };

        template<typename T>
        struct DataHolder : DataHolderBase, std::tuple<T> {
            using std::tuple<T>::tuple;
            size_t id() override {
                return TOF<T>();
            }
            
            T& get() {
                return std::get<0>(*this);
            }

            DataHolderBase* clone() override {
                //static_assert(std::is_move_constructible<T>::value);
                //static_assert(std::is_copy_constructible<T>::value);
                return new DataHolder<T> {get()};
            }
        };

        DataHolderBase* val;

    public:
        void swap(KAny& rh) {
            auto* tmp = val;
            val = rh.val;
            rh.val = tmp;
        }

        KAny():val(nullptr) {

        }

        bool empty() {
            return val == nullptr;
        }
        
        KAny(KAny&& rh): val(rh.val) {
            rh.val = nullptr;
        }

        KAny& operator =(const KAny& rh) {
            if (val) {
                delete val;
            }
            if (rh.val) {
                val = rh.val->clone();
            }
            return *this;
        }

        KAny(const KAny& rh) : val(rh.val->clone()) {
        }

        template<typename T>
        KAny(T&& val) {
            if (&typeid(val) != &typeid(std::nullptr_t)) {
                this->val = new DataHolder<typename std::decay<T>::type>(std::forward<T>(val));
            }
            else {
                this->val = nullptr;
            }
        }

        template<typename T>
        KAny(const T& val):val(new DataHolder<T>(val)) {
            if (&typeid(val) != &typeid(std::nullptr_t)) {
                this->val = new DataHolder<typename std::decay<T>::type>(std::forward<T>(val));
            }
            else {
                this->val = nullptr;
            }
        }

        ~KAny() {
            if (val != nullptr) {
                delete val;
            }
        }

        template<typename T>
        bool is() {
            if (val != nullptr) {
                return val->is<T>();
            }
            return false;
        }

        template<typename T> 
        inline T* get() {
            if (val != nullptr && val->id() == TOF<T>()) {
                auto* data = static_cast<DataHolder<T>*>(this->val);
                return &data->get();
            }
            return nullptr;
        }
    };
}