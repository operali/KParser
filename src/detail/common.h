// author: operali
// desc: object release detecting, turn on KOBJECT_DEBUG to see what is left unreleased


#pragma once

#include "./base.h"
#include "./conf.h"
#include "./text.h"

namespace KLib42 {
    // count objects in runtime, KObject::count should be 0 after destruction;
    struct KObject {
        static KUSIZE count;
#ifdef KOBJECT_DEBUG
        static std::vector<KObject*> all;
        inline static std::vector<KObject*>& debug() {
            return all;
        }
#endif

        
        KObject() {
#ifdef KOBJECT_DEBUG
            all.push_back(this);
#endif
            KObject::count++;
        }
        virtual ~KObject() {
#ifdef KOBJECT_DEBUG
            all.erase(std::remove(all.begin(), all.end(), this));
#endif
            KObject::count--;
        }
    };

    
    struct KError : public KObject {
        virtual ~KError() {};
        virtual std::string message() = 0;
    };
}

