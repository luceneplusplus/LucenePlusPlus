/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "StandardTokenizerInterface.h"

namespace Lucene
{
    /// This character denotes the end of file
    const int32_t StandardTokenizerInterface::YYEOF = -1;
    
    StandardTokenizerInterface::~StandardTokenizerInterface()
    {
    }
}
