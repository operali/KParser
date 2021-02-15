#include "./error.h"
#include <sstream>
#include "./dsl.h"

namespace KLib42 {
    KShared<IRange> SyntaxError::getRange() {
        auto loc = source->getLocation(location);
        return loc->getRange();
    }

    std::string SyntaxError::message() {
        std::stringstream ss;
        auto iloc = source->getLocation(location);
        auto loc = iloc->location();
        ss << "fail to parse at (" << loc.row << ":" << loc.col << ")" << std::endl;
        if (loc.row != 0) {
            auto lno = loc.row - 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }
        
        auto buff = source->buff();
        auto left = buff + loc.left;
        auto mid = buff + loc.idx;
        auto right = buff + loc.right;
        ss << loc.row << "| " << std::string(left, mid)
        << "^^^"
        << std::string(mid, right) << std::endl;

        if (loc.row+1 != source->lineCount()) {
            auto lno = loc.row + 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }
return ss.str();
    };

    KShared<IRange> IDError::getRange() {
        return KShared<IRange>{};
    }


    KShared<IRange> RedefinedIDError::getRange() {
        return KShared<IRange>{};
    }

    std::string RedefinedIDError::message() {
        std::stringstream ss;
        ss << "redefined id of " << this->id->name << std::endl;
        auto locLeft = this->id->matcher->location();
        auto locRight = locLeft + this->id->matcher->length();
        auto ilocLeft = this->source->getLocation(locLeft);
        auto loc = ilocLeft->location();
        if (loc.row != 0) {
            auto lno = loc.row - 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }

        auto buff = source->buff();
        auto left = buff + loc.left;
        auto mid = buff + loc.idx;
        auto mid1 = buff + locRight;
        auto right = buff + loc.right;
        ss << loc.row << "| " << std::string(left, mid)
            << "^^^"
            << std::string(mid, mid1)
            << "^^^"
            << std::string(mid1, right) << std::endl;

        if (loc.row + 1 != source->lineCount()) {
            auto lno = loc.row + 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }
        return ss.str();
    };

    KShared<IRange> UndefinedIDError::getRange() {
        return KShared<IRange>{};
    }

    std::string UndefinedIDError::message() {
        std::stringstream ss;
        ss << "undefined id of " << this->id->name << std::endl;
        auto locLeft = this->id->matcher->location();
        auto locRight = locLeft + this->id->matcher->length();
        auto ilocLeft = this->source->getLocation(locLeft);
        auto loc = ilocLeft->location();
        if (loc.row != 0) {
            auto lno = loc.row - 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }

        auto buff = source->buff();
        auto left = buff + loc.left;
        auto mid = buff + loc.idx;
        auto mid1 = buff + locRight;
        auto right = buff + loc.right;
        ss << loc.row << "| " << std::string(left, mid)
            << "^^^"
            << std::string(mid, mid1)
            << "^^^"
            << std::string(mid1, right) << std::endl;

        if (loc.row + 1 != source->lineCount()) {
            auto lno = loc.row + 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }
        return ss.str();
    };

    KShared<IRange> RecursiveIDError::getRange() {
        return KShared<IRange>{};
    }

    std::string RecursiveIDError::message() {
        std::stringstream ss;
        ss << "recursive id defination of " << this->id->name << std::endl;
        auto locLeft = this->id->matcher->location();
        auto locRight = locLeft + this->id->matcher->length();
        auto ilocLeft = this->source->getLocation(locLeft);
        auto loc = ilocLeft->location();
        if (loc.row != 0) {
            auto lno = loc.row - 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }

        auto buff = source->buff();
        auto left = buff + loc.left;
        auto mid = buff + loc.idx;
        auto mid1 = buff + locRight;
        auto right = buff + loc.right;
        ss << loc.row << "| " << std::string(left, mid)
            << "^^^"
            << std::string(mid, mid1)
            << "^^^"
            << std::string(mid1, right) << std::endl;

        if (loc.row + 1 != source->lineCount()) {
            auto lno = loc.row + 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }
        return ss.str();
    };
}