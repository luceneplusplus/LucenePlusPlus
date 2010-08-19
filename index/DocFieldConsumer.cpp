/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DocFieldConsumer.h"

namespace Lucene
{
    DocFieldConsumer::~DocFieldConsumer()
    {
    }
    
    void DocFieldConsumer::setFieldInfos(FieldInfosPtr fieldInfos)
    {
        this->fieldInfos = fieldInfos;
    }
}
