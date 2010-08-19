/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI DocFieldConsumerPerThread : public LuceneObject
	{
	public:
		virtual ~DocFieldConsumerPerThread();
		
		LUCENE_CLASS(DocFieldConsumerPerThread);
	
	public:
		virtual void startDocument() = 0;
		virtual DocWriterPtr finishDocument() = 0;
		virtual DocFieldConsumerPerFieldPtr addField(FieldInfoPtr fi) = 0;
		virtual void abort() = 0;
	};
}
