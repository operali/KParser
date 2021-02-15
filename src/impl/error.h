#pragma once
#include "../KParser.h"

namespace KLib42 {
    struct IRange;
    struct DSLID;

    struct KParserError : public KError {
        Parser* impl;
        KParserError(Parser* impl) :impl(impl) {}
        virtual KShared<IRange> getRange() = 0;
    };

    struct SyntaxError : public KParserError {
        KUSIZE location;
        SyntaxError(Parser* impl, KUSIZE location):KParserError(impl), location(location) {
        }
        
        KShared<IRange> getRange() override;
        std::string message() override;
    };

    struct IDError : public KParserError {
        DSLID* id;
        IDError(Parser* impl, DSLID* id) :KParserError{ impl }, id(id) {}

        KShared<IRange> getRange() override;
    };

    struct RedefinedIDError : public IDError {
        RedefinedIDError(Parser* impl, DSLID* id) :IDError(impl, id) {}
        KShared<IRange> getRange() override;

        std::string message() override;
    };

    struct UndefinedIDError : public IDError {
        UndefinedIDError(Parser* impl, DSLID* id) :IDError(impl, id) {}
        KShared<IRange> getRange() override;

        std::string message() override;
    };

    struct RecursiveIDError : public IDError {
        RecursiveIDError(Parser* impl, DSLID* id) :IDError(impl, id) {}
        KShared<IRange> getRange() override;

        std::string message() override;
    };
}