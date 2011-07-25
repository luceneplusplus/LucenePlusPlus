/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TermCollectingRewrite.h"
#include "FilteredTermEnum.h"
#include "Term.h"

namespace Lucene
{
    TermCollectingRewrite::~TermCollectingRewrite()
    {
    }

    void TermCollectingRewrite::collectTerms(IndexReaderPtr reader, MultiTermQueryPtr query, TermCollectorPtr collector)
    {
        FilteredTermEnumPtr enumerator(query->getEnum(reader));
        LuceneException finally;
        try
        {
            do
            {
                TermPtr t(enumerator->term());
                if (!t || !collector->collect(t, enumerator->difference()))
                    break;
            }
            while (enumerator->next());
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        enumerator->close();
        finally.throwException();
    }

    TermCollector::~TermCollector()
    {
    }
}
