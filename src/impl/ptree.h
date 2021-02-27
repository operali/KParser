#pragma once
#include <memory>
#include <string>
#include <vector>
#include "./ref.h"
#include "./any.h"


namespace KLib42 {
    
    struct KProperty;

    struct KDocument {
        
        KDocument() = default;
        ~KDocument();
        KProperty* createElement();
        
        template<typename T>
        KProperty* createElement(T&& v);

        KProperty* createArray();
        KProperty* createObject();
    private:
        std::vector<KProperty*> pool;
    };

    struct KProperty {
    private:
        struct KPropertyImpl* pImpl;
    public:
        enum class ETYPE {
            element = 0,
            array,
            object
        };
    private:
        ETYPE _type;
        KProperty() = delete;
        KProperty(ETYPE type);
    public:
        ~KProperty();
        friend struct KDocument;
        inline bool isObject() {
            return _type == ETYPE::object;
        }
        KProperty* getByNameRaw(const std::string& name);
        KProperty* addByNameRaw(const std::string& name, KProperty* val);
        
        
        inline bool isArray() {
            return _type == ETYPE::array;
        }
        int KProperty::arraySize();
        KProperty* getByIndexRaw(size_t idx);
        KProperty* addRaw(KProperty* val);

        inline bool isElement() {
            return _type == ETYPE::element;
        }

        KAny* getPropAny();
        template<typename T>
        T* get() {
            KAny* r = getPropAny();
            static_assert(!std::is_reference<T>::value, "KProperty set must not be reference");
            if (!r) {
                return nullptr;
            }
            else {
                return r->get<T>();
            }
        }
        
        void setAny(KAny&& val);

        template<typename T>
        void set(T&& t) {
            return setAny(KAny(t));
        }

    public:
        template<typename T>
        T* getByIndex(size_t idx) {
            auto r = getByIndexRaw(idx);
            return r->get<T>();
        }

        template<typename T>
        KProperty* add(KDocument& doc, T&& v) {
            auto* elem = doc.createElement();
            elem->set(v);
            return addRaw(elem);
        }

        template<typename T>
        T* getByName(const std::string& name) {
            auto* p = getByNameRaw(name);
            return p->get<T>();
        }
        
        template<typename T>
        void setByName(const std::string& name, T&& val) {
            p->setRaw<T>(name, val);
        }
    };

    template<typename T>
    KProperty* KDocument::createElement(T&& v) {
        auto* elem = createElement();
        elem->set<T>(std::move(v));
        return elem;
    }
}

