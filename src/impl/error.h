// author: operali
// desc: structral error

#pragma once
#include "./common.h"

namespace KLib42 {
    struct IRange;
    struct DSLID;
    struct Rule;

    struct KParserError : public KError {
        KShared<ISource> source;
        KParserError(KShared<ISource> source) :source(source) {}
        virtual std::pair<size_t, size_t> getRange() = 0;
    };

    struct SyntaxError : public KParserError {
        KUSIZE location;
        Rule* rule;
        SyntaxError(KShared<ISource> source, Rule* r, KUSIZE location):KParserError(source), rule(r), location(location) {
        }
        
        std::pair<size_t, size_t> getRange() override;
        std::string message() override;
    };

    struct IDError : public KParserError {
        DSLID* id;
        
        std::pair<size_t, size_t> getRange() override;
        IDError(KShared<ISource> source, DSLID* id) :KParserError{ source }, id(id) {}
    };

    struct RedefinedIDError : public IDError {
        RedefinedIDError(KShared<ISource> source, DSLID* id) :IDError(source, id) {}
        
        std::string message() override;
    };

    struct UndefinedIDError : public IDError {
        UndefinedIDError(KShared<ISource> source, DSLID* id) :IDError(source, id) {}
        
        std::string message() override;
    };

    struct RecursiveIDError : public IDError {
        RecursiveIDError(KShared<ISource> source, DSLID* id) :IDError(source, id) {}
        
        std::string message() override;
    };
}