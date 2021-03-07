// author: operali
// desc: structral error
#include <sstream>
#include <iomanip>

#include "./error.h"
#include "./ebnf.h"

namespace KLib42 {

    std::string genErrorMessage(const std::string& desc, ISource* source, std::pair<size_t, size_t> range) {
        std::stringstream ss;

        auto locLeft = range.first;
        auto locRight = range.second;
        auto ilocLeft = source->getLocation(locLeft);
        auto loc = ilocLeft->location();
        ss << desc << "(" << loc.row << "," << loc.col << ")" << "\n";
        
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
            << std::string(mid, mid1)
            << std::string(mid1, right) << std::endl;
        ss << std::setw(mid - left + 4) << "^" << std::setw(mid1 - mid) << "^" << std::endl;
        if (loc.row + 1 != source->lineCount()) {
            auto lno = loc.row + 1;
            auto iline = source->getLine(lno);
            if (iline) {
                ss << lno << "| " << iline->str() << std::endl;
            }
        }
        return ss.str();
    }

    std::pair<size_t, size_t> SyntaxError::getRange() {
        auto loc = source->getLocation(location);
        return loc->getRange()->range();
    }

    std::pair<size_t, size_t> IDError::getRange() {
        return id->range->range();
    }

    std::string SyntaxError::message() {
        return genErrorMessage("syntax error", source.get(), getRange());
    };

    
    std::string RedefinedIDError::message() {
        return genErrorMessage("redefine ID of "+id->name, source.get(), getRange());
    };


    std::string UndefinedIDError::message() {
        return genErrorMessage("undefine ID of "+id->name, source.get(), getRange());
    };

    std::string RecursiveIDError::message() {
        return genErrorMessage("define ID recursively of " + id->name, source.get(), getRange());
    };
}