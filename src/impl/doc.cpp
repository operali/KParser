#include <cassert>
#include "./doc.h"

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

	KProperty* KProperty::setByNameRaw(const std::string& name, KProperty* prop, bool check) {
		assert(isObject(), "KProperty::addByName must be object");
		auto& vals = static_cast<PropNodeObjectImpl*>(pImpl)->values;
		
		if (check) {
			for (auto& v : vals) {
				if (v.first == name) {
					return nullptr;
				}
			}
		}

		vals.push_back(std::make_pair(name, prop));
		return this;
	}

	KProperty* KProperty::getByIndexRaw(size_t idx) {
		if (isArray()) {
			auto& vals = static_cast<PropNodeArrayImpl*>(pImpl)->values;
			return vals.at(idx);
		}
		else if (isObject()) {
			auto& vals = static_cast<PropNodeObjectImpl*>(pImpl)->values;
			return vals.at(idx).second;
		}
		else {
			return nullptr;
		}
	}

	KProperty* KProperty::setRaw(KProperty* val) {
		assert(isArray(), "KProperty::add must be object");

		auto& vals = static_cast<PropNodeArrayImpl*>(pImpl)->values;
		vals.emplace_back(val);
		return this;
	}

	int KProperty::arraySize() {
		// assert(isArray(), "KProperty::arraySize must be object");
		if (isArray()) {
			auto& vals = static_cast<PropNodeArrayImpl*>(pImpl)->values;
			return vals.size();
		}
		else if (isObject()) {
			auto& vals = static_cast<PropNodeObjectImpl*>(pImpl)->values;
			return vals.size();
		}
		else {
			return -1;
		}
	}

	KAny* KProperty::getRaw() {
		assert(isElement(), "KProperty::getPropAny must be property");
		auto prop = static_cast<PropNodePropImpl*>(pImpl);
		return &prop->value;
	}

	KProperty* KProperty::getByPathRaw(const std::string& path) {
		char* pstr = const_cast<char*>(path.c_str());
		const char* dem = "/";
		const char* tok = strtok(pstr, dem);
		KProperty* curP = this;
		do{
			if (!curP->isObject()) {
				return nullptr;
			}
			curP = curP->getByNameRaw(tok);
			if (curP == nullptr) {
				return nullptr;
			}
			tok = strtok(nullptr, dem);
		} while (tok != nullptr);
		return curP;
	}

	KProperty* KProperty::setByPathRaw(KDocument& doc, const std::string& path, KProperty* p) {
		char* pstr = const_cast<char*>(path.c_str());
		const char* dem = "/";
		KProperty* curP = this;
		
		const char* tok = strtok(pstr, dem);
		std::string pathTok = tok;

		tok = strtok(nullptr, dem);
		while(true) {
			if (tok == nullptr) {
				if (!curP->setByNameRaw(pathTok, p, true)) {
					return nullptr;
				}
				return p;
			}
			if (!curP->isObject()) {
				return nullptr;
			}
			auto* lastP = curP;
			curP = curP->getByNameRaw(pathTok);
			if (curP == nullptr) {
				curP = doc.createObject();
				if (!lastP->setByNameRaw(pathTok, curP, true)) {
					return nullptr;
				}
			}
			pathTok = tok;
			tok = strtok(nullptr, dem);
		};
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

	void KProperty::visit(std::function<void(std::vector<KFrame>&, KFrame&)> handle) {
		/** curP, state { sink, iter, float } **/
		std::vector<KFrame> vec;
		KFrame curF(this);
		
		while (true)
		{
			auto* curP = curF.p;
			auto index = curF.index;
			auto st = curF.st;
			if (st == EVISIT::sink) {
				handle(vec, curF);
				if (curP->isElement()) {
					curF.st = EVISIT::float_;
				}
				else {
					curF.st = EVISIT::iter;
				}
			}
			else if (st == EVISIT::iter) {
				auto sz = curP->arraySize();
				if (sz == index) {
					curF.st = EVISIT::float_;
				}
				else {
					auto* p = curP->getByIndexRaw(index);
					vec.push_back(curF);
					curF = KFrame(p);
				}
			}
			else {//if (st == EVISIT::float_) {
				handle(vec, curF);
				if (vec.empty()) {
					break;
				}
				curF = vec.back();
				curF.index++;
				vec.pop_back();
			}
		}
	}

	std::string KProperty::toJson() {
		size_t margin = -1;
		std::stringstream ss;
		visit([&](std::vector<KFrame>& h, KFrame& frame) {
			auto* parent = h.empty() ? nullptr : &h.back();
			auto* curP = frame.p;
			auto index = frame.index;
			auto st = frame.st;

			KProperty* pp;
			int pidx;
			EVISIT pst;
			if (parent) {
				pp = parent->p;
				pidx = parent->index;
				pst = parent->st;
			}
			if (st == EVISIT::sink) {
				margin++;
				for (size_t i = 0; i < margin; ++i) {
					ss << "  ";
				}
				if (parent) {
					if (pp->isObject()) {
						auto& vals = static_cast<PropNodeObjectImpl*>(pp->pImpl)->values;
						ss << "\"" << vals.at(pidx).first << "\"" << ":";
					}
				}
				if (curP->isElement()) {
					auto* r = curP->getRaw();
					if (r->is<int>() || r->is<size_t>() || r->is<float>() || r->is<float>()) {
						ss << curP->getRaw()->toString();
					}
					else if (r->is<bool>()) {
						auto b = *r->get<bool>();
						if (b) {
							ss << "true";
						}
						else {
							ss << "false";
						}
					}
					else if (r->is<std::string>()) {
						ss << curP->getRaw()->toString();
					}
					else {
						ss << "\"" << curP->getRaw()->toString() << "\"";
					}
				}
				else if (curP->isArray()) {
					ss << "[";
					ss << "\n";
				}
				else if (curP->isObject()) {
					ss << "{";
					ss << "\n";
				}
			}
			else {
				if (curP->isArray()) {
					for (size_t i = 0; i < margin; ++i) {
						ss << "  ";
					}
					ss << "]";
				}
				else if (curP->isObject()) {
					for (size_t i = 0; i < margin; ++i) {
						ss << "  ";
					}
					ss << "}";
				}
				if (parent) {
					if (pidx != pp->arraySize() - 1) {
						ss << ",";
					}
					ss << "\n";
				}
				margin--;
			}
		});
		return ss.str();
	}

} 