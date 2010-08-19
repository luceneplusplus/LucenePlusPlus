/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IndexCommit.h"
#include "Directory.h"

namespace Lucene
{
    IndexCommit::~IndexCommit()
    {
    }
    
    void IndexCommit::deleteCommit()
    {
        boost::throw_exception(UnsupportedOperationException(L"This IndexCommit does not support this method."));
    }
    
    bool IndexCommit::isDeleted()
    {
        boost::throw_exception(UnsupportedOperationException(L"This IndexCommit does not support this method."));
        return false;
    }
    
    bool IndexCommit::isOptimized()
    {
        boost::throw_exception(UnsupportedOperationException(L"This IndexCommit does not support this method."));
        return false;
    }
    
    bool IndexCommit::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        IndexCommitPtr otherCommit(boost::dynamic_pointer_cast<IndexCommit>(other));
        if (!otherCommit)
            return false;
        return (otherCommit->getDirectory()->equals(getDirectory()) && otherCommit->getVersion() == getVersion());
    }
    
    int32_t IndexCommit::hashCode()
    {
        return getDirectory()->hashCode() + StringUtils::hashCode(getSegmentsFileName());
    }
    
    int64_t IndexCommit::getVersion()
    {
        boost::throw_exception(UnsupportedOperationException(L"This IndexCommit does not support this method."));
        return 0;
    }
    
    int64_t IndexCommit::getGeneration()
    {
        boost::throw_exception(UnsupportedOperationException(L"This IndexCommit does not support this method."));
        return 0;
    }
    
    int64_t IndexCommit::getTimestamp()
    {
        return getDirectory()->fileModified(getSegmentsFileName());
    }
    
    MapStringString IndexCommit::getUserData()
    {
        boost::throw_exception(UnsupportedOperationException(L"This IndexCommit does not support this method."));
        return MapStringString();
    }
}
