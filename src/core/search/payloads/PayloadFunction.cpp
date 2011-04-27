/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "PayloadFunction.h"
#include "Explanation.h"

namespace Lucene
{
    PayloadFunction::PayloadFunction()
    {
    }

    PayloadFunction::~PayloadFunction()
    {
    }
    
    ExplanationPtr PayloadFunction::explain(int32_t docId, int32_t numPayloadsSeen, double payloadScore)
    {
        ExplanationPtr result(newLucene<Explanation>());
        result->setDescription(L"Unimpl Payload Function Explain");
        result->setValue(1);
        return result;
    }
}
