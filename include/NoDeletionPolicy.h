/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef NODELETIONPOLICY_H
#define NODELETIONPOLICY_H

#include "IndexDeletionPolicy.h"

namespace Lucene
{
    /// An {@link IndexDeletionPolicy} which keeps all index commits around, never deleting them. 
    /// This class is a singleton and can be accessed by referencing {@link #INSTANCE}.
    class LPPAPI NoDeletionPolicy : public IndexDeletionPolicy
    {
    public:
        virtual ~NoDeletionPolicy();
        
        LUCENE_CLASS(NoDeletionPolicy);
    
    public:
        /// The single instance of this class.
        static IndexDeletionPolicyPtr INSTANCE();
        
        virtual void onInit(Collection<IndexCommitPtr> commits);
        virtual void onCommit(Collection<IndexCommitPtr> commits);
    };
}

#endif
