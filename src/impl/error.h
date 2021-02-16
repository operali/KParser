#pragma once
#include "./common.h"

namespace KLib42 {
    struct IRange;
    struct DSLID;

    struct KParserError : public KError {
        KShared<ISource> source;
        KParserError(KShared<ISource> source) :source(source) {}
        virtual KUnique<IRange> getRange() = 0;
    };

    struct SyntaxError : public KParserError {
        KUSIZE location;
        SyntaxError(KShared<ISource> source, KUSIZE location):KParserError(source), location(location) {
        }
        
        KUnique<IRange> getRange() override;
        std::string message() override;
    };

    struct IDError : public KParserError {
        DSLID* id;
        IDError(KShared<ISource> source, DSLID* id) :KParserError{ source }, id(id) {}

        KUnique<IRange> getRange() override;
    };

    struct RedefinedIDError : public IDError {
        RedefinedIDError(KShared<ISource> source, DSLID* id) :IDError(source, id) {}
        KUnique<IRange> getRange() override;

        std::string message() override;
    };

    struct UndefinedIDError : public IDError {
        UndefinedIDError(KShared<ISource> source, DSLID* id) :IDError(source, id) {}
        KUnique<IRange> getRange() override;

        std::string message() override;
    };

    struct RecursiveIDError : public IDError {
        RecursiveIDError(KShared<ISource> source, DSLID* id) :IDError(source, id) {}
        KUnique<IRange> getRange() override;

        std::string message() override;
    };
}