#include "./error.h"
#include <sstream>
#include "./dsl.h"

namespace KLib42 {
    KUnique<IRange> SyntaxError::getRange() {
        auto loc = source->getLocation(location);
        return loc->getRange();
    }

    std::string SyntaxError::message() {
        std::stringstream ss;
        auto iloc = source->getLocation(location);
        auto loc = iloc->location();
        ss << "location(" << loc.row << "," << loc.col << ")";
        ss<< ":syntax error"<< std::endl;
        if (loc.row != 0) {
            auto lno = loc.row - 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }
        
        auto buff = source->raw();
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

    KUnique<IRange> IDError::getRange() {
        return KUnique<IRange>{};
    }


    KUnique<IRange> RedefinedIDError::getRange() {
        return KUnique<IRange>{};
    }

    std::string RedefinedIDError::message() {
        std::stringstream ss;

        auto range = this->id->range->range();
        auto locLeft = range.first;
        auto locRight = range.second;
        auto ilocLeft = this->source->getLocation(locLeft);
        auto loc = ilocLeft->location();
        ss << "location(" << loc.row << "," << loc.col << ")";
        ss << "redefined id of " << this->id->name << std::endl;

        if (loc.row != 0) {
            auto lno = loc.row - 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }

        auto buff = source->raw();
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

    KUnique<IRange> UndefinedIDError::getRange() {
        return KUnique<IRange>{};
    }

    std::string UndefinedIDError::message() {
        std::stringstream ss;
        auto range = this->id->range->range();
        auto locLeft = range.first;
        auto locRight = range.second;
        auto ilocLeft = this->source->getLocation(locLeft);
        auto loc = ilocLeft->location();
        ss << "location(" << loc.row << "," << loc.col << ")";
        ss << "undefined id of " << this->id->name << std::endl;

        if (loc.row != 0) {
            auto lno = loc.row - 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }

        auto buff = source->raw();
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

    KUnique<IRange> RecursiveIDError::getRange() {
        return KUnique<IRange>{};
    }

    std::string RecursiveIDError::message() {
        std::stringstream ss;
        auto range = this->id->range->range();
        auto locLeft = range.first;
        auto locRight = range.second;
        auto ilocLeft = this->source->getLocation(locLeft);
        auto loc = ilocLeft->location();
        ss << "location (" << loc.row << ", " << loc.col << ")";
        ss << "recursive id defination of " << this->id->name << std::endl;

        if (loc.row != 0) {
            auto lno = loc.row - 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }

        auto buff = source->raw();
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