/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PrefixFilter.h"
#include "PrefixQuery.h"
#include "Term.h"

namespace Lucene
{
    PrefixFilter::PrefixFilter(TermPtr prefix) : MultiTermQueryWrapperFilter(newLucene<PrefixQuery>(prefix))
    {
    }
    
    PrefixFilter::~PrefixFilter()
    {
    }
    
    TermPtr PrefixFilter::getPrefix()
    {
        return boost::static_pointer_cast<PrefixQuery>(query)->getPrefix();
    }
    
    String PrefixFilter::toString()
    {
        StringStream buffer;
        buffer << L"PrefixFilter(" << getPrefix()->toString() << L")";
        return buffer.str();	
    }
}
