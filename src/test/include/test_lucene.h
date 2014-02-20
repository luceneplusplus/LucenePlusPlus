/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TEST_LUCENE_H
#define TEST_LUCENE_H

#include "Lucene.h"
#include "LuceneContrib.h"
#include "StringUtils.h"
#include <gtest/gtest.h>

namespace std {

inline std::ostream& operator<< (std::ostream& out, const Lucene::String& s) {
    out << Lucene::StringUtils::toUTF8(s);
    return out;
}

}

namespace Lucene {

DECLARE_SHARED_PTR(MockDirectoryFailure)
DECLARE_SHARED_PTR(MockFSDirectory)
DECLARE_SHARED_PTR(MockLock)
DECLARE_SHARED_PTR(MockLockFactory)
DECLARE_SHARED_PTR(MockRAMDirectory)
DECLARE_SHARED_PTR(MockRAMInputStream)
DECLARE_SHARED_PTR(MockRAMOutputStream)
DECLARE_SHARED_PTR(MockFilter)

typedef HashMap<String, FieldPtr> MapStringField;

struct check_exception {
    check_exception(LuceneException::ExceptionType type) : checkType(type) {}
    inline bool operator()(const LuceneException& e) {
        return (checkType == LuceneException::Null || e.getType() == checkType);
    }
    LuceneException::ExceptionType checkType;
};

}

#endif
