/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbstractAllTermDocs.h"

namespace Lucene
{
	class LPPAPI AllTermDocs : public AbstractAllTermDocs
	{
	public:
		AllTermDocs(SegmentReaderPtr parent);
		virtual ~AllTermDocs();
		
		LUCENE_CLASS(AllTermDocs);
	
	protected:
		BitVectorWeakPtr _deletedDocs;
		
	public:
	    virtual bool isDeleted(int32_t doc);
	};
}
