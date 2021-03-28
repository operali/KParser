// author: operali
// desc: structral TEXT object

#pragma once
#include <string>
#include "./ref.h"
namespace KLib42 {
    struct ISource;
    struct ILocation;
    struct IRange;
    struct ILine;

    struct ITextObject {
        virtual ~ITextObject() = default;
        virtual ISource* getRawSource() = 0;
    };

    struct ISource {
        // raw text
        virtual const char* raw() = 0;
        virtual size_t lineCount() = 0;
        virtual size_t len() = 0;
        
        virtual void setText(const std::string& text) = 0;

        virtual KUnique<ILine> getLine(size_t index) = 0;
        virtual KUnique<ILocation> getLocation(size_t index) = 0;
        virtual KUnique<IRange> getRange(size_t from, size_t to) = 0;
        virtual ~ISource() = default;
    };

    struct loc {
        size_t row;
        size_t col;
        size_t left;
        size_t right;
        size_t idx;
    };

    struct ILocation : public ITextObject {
        virtual size_t index() = 0;
        virtual loc location() = 0;
        virtual KUnique<ILine> getLine() = 0;
        virtual KUnique<IRange> getRange() = 0;
    };

    struct IRange : public ITextObject {
        virtual std::pair<size_t, size_t> range() = 0;
        virtual std::string str() = 0;
    };

    struct ILine : public ITextObject {
        virtual size_t index() = 0;
        virtual std::string str() = 0;
        virtual KUnique<ILine> previous() = 0;
        virtual KUnique<ILine> next() = 0;
        virtual KUnique<IRange> toRange() = 0;
    };

    struct KSourceImpl;
    struct KSource : ISource {
        KSourceImpl* pImpl;
        KSource();
        ~KSource();
        
        const char* raw() override;
        size_t len() override;
        size_t lineCount() override;

        void setText(const std::string& text) override;
        KUnique<ILine> getLine(size_t index) override;
        KUnique<ILocation> getLocation(size_t index) override;
        KUnique<IRange> getRange(size_t from, size_t to) override;
    };
}