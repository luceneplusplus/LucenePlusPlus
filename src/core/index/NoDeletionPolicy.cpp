/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "NoDeletionPolicy.h"

namespace Lucene
{
    NoDeletionPolicy::~NoDeletionPolicy()
    {
    }
    
    IndexDeletionPolicyPtr NoDeletionPolicy::INSTANCE()
    {
        static IndexDeletionPolicyPtr _INSTANCE;
        if (!_INSTANCE)
        {
            _INSTANCE = newLucene<NoDeletionPolicy>();
            CycleCheck::addStatic(_INSTANCE);
        }
        return _INSTANCE;
    }
    
    void NoDeletionPolicy::onInit(Collection<IndexCommitPtr> commits)
    {
    }
    
    void NoDeletionPolicy::onCommit(Collection<IndexCommitPtr> commits)
    {
    }
}
