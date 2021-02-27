#include <cassert>
#include "./ptree.h"

namespace KLib42 {
	struct KPropertyImpl {
		virtual ~KPropertyImpl(){}
	};

	struct PropNodePropImpl : public KPropertyImpl {
		KAny value;
	};

	struct PropNodeArrayImpl : public KPropertyImpl {
		std::vector<KProperty*> values;
	};

	struct PropNodeObjectImpl : public KPropertyImpl {
		std::vector<std::pair<std::string, KProperty*>> values;
	};

	KProperty::KProperty(ETYPE type):_type(type) {
		if (_type == ETYPE::array) {
			pImpl = new PropNodeArrayImpl();
		} else if (_type == ETYPE::object) {
			pImpl = new PropNodeObjectImpl();
		} else // (_type == ETYPE::e_prop) 
		{
			pImpl = new PropNodePropImpl();
		}
	}

	KProperty::~KProperty() {
		delete pImpl;
	}
	
	KProperty* KProperty::getByNameRaw(const std::string& name) {
		assert(isObject(), "KProperty::getByName must be object");
		auto& vals = static_cast<PropNodeObjectImpl*>(pImpl)->values;
		using IT = std::decay_t<decltype(vals)>::iterator;
		using E = std::pair < std::string, KProperty*>;
		IT it = std::find_if(vals.begin(), vals.end(), [&](E& el) {
			return el.first == name;
		});
		if (it == vals.end()) {
			return nullptr;
		}
		return it->second;
	}

	KProperty* KProperty::addByNameRaw(const std::string& name, KProperty* prop) {
		assert(isObject(), "KProperty::addByName must be object");
		auto& vals = static_cast<PropNodeObjectImpl*>(pImpl)->values;
		// note, didnot check exist
		vals.push_back(std::make_pair(name, prop));
		return this;
	}

	KProperty* KProperty::getByIndexRaw(size_t idx) {
		assert(isArray(), "KProperty::getByName must be object");
		
		auto& vals = static_cast<PropNodeArrayImpl*>(pImpl)->values;
		return vals.at(idx);
	}

	KProperty* KProperty::addRaw(KProperty* val) {
		assert(isArray(), "KProperty::add must be object");

		auto& vals = static_cast<PropNodeArrayImpl*>(pImpl)->values;
		vals.emplace_back(val);
		return this;
	}

	int KProperty::arraySize() {
		assert(isArray(), "KProperty::arraySize must be object");
		
		auto& vals = static_cast<PropNodeArrayImpl*>(pImpl)->values;
		return vals.size();
	}

	KAny* KProperty::getPropAny() {
		assert(isElement(), "KProperty::getPropAny must be property");
		auto prop = static_cast<PropNodePropImpl*>(pImpl);
		return &prop->value;
	}

	void KProperty::setAny(KAny&& val) {
		assert(isElement(), "KProperty::setAny must be property");
		auto* prop = static_cast<PropNodePropImpl*>(pImpl);
		prop->value = val;
	}

	KDocument::~KDocument() {
		for (auto* p : pool) {
			delete p;
		}
	}
	
	KProperty* KDocument::createElement() {
		auto* p = new KProperty(KProperty::ETYPE::element);
		pool.push_back(p);
		return p;
	}

	KProperty* KDocument::createArray() {
		auto* p = new KProperty(KProperty::ETYPE::array);
		pool.push_back(p);
		return p;
	}

	KProperty* KDocument::createObject() {
		auto* p = new KProperty(KProperty::ETYPE::object);
		pool.push_back(p);
		return p;
	}
} 