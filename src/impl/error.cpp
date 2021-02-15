#include "./error.h"

namespace KLib42 {
    KShared<IRange> SyntaxError::getRange() {
        return KShared<IRange>{};
    }

    std::string SyntaxError::message() {
        return "unknown";
    };

    KShared<IRange> IDError::getRange() {
        return KShared<IRange>{};
    }


    KShared<IRange> RedefinedIDError::getRange() {
        return KShared<IRange>{};
    }

    std::string RedefinedIDError::message() {
        return "unknown";
    };

    KShared<IRange> UndefinedIDError::getRange() {
        return KShared<IRange>{};
    }

    std::string UndefinedIDError::message() {
        return "unknown";
    };

    KShared<IRange> RecursiveIDError::getRange() {
        return KShared<IRange>{};
    }

    std::string RecursiveIDError::message() {
        return "unknown";
    };
}