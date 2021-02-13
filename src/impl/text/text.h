
#pragma once
#include "../ref.h"

namespace KLib42 {
    struct ITextObject {
        virtual ~ITextObject() = default;
    };

    struct ISource;
    struct ILocation;
    struct IRange;
    struct ILine;

    struct ISource : public ITextObject {
        virtual const char* buff() = 0;
        virtual KShared<IEnumerator<ILine>> lines() = 0;
        virtual uint32_t lineCount() = 0;
        virtual uint32_t len() = 0;
        virtual KShared<ILine> getLine(size_t index) = 0;
        virtual KShared<ILocation> getLocation(size_t index) = 0;
    };

    struct loc {
        uint32_t row;
        uint32_t col;
    };

    struct ILocation : public ITextObject {
        virtual uint32_t index() = 0;
        virtual loc location() = 0;
        virtual KShared<ILine> getLine() = 0;
    };

    struct IRange : public ITextObject {
        virtual size_t from() = 0;
        virtual size_t to() = 0;
        virtual std::string str() = 0;
    };

    struct ILine : public ITextObject {
        virtual uint32_t index() = 0;
        virtual std::string str() = 0;
        virtual KShared<IRange> toRange() = 0;
    };

    struct KTextImpl;
    struct KText {
        KTextImpl* pImpl;
        KText();
        void setText(const std::string& text);
        KShared<ISource> getSource();
    };
}