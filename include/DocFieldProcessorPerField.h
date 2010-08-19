/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	/// Holds all per thread, per field state.
	class LPPAPI DocFieldProcessorPerField : public LuceneObject
	{
	public:
		DocFieldProcessorPerField(DocFieldProcessorPerThreadPtr perThread, FieldInfoPtr fieldInfo);
		virtual ~DocFieldProcessorPerField();
		
		LUCENE_CLASS(DocFieldProcessorPerField);
				
	public:
		DocFieldConsumerPerFieldPtr consumer;
		FieldInfoPtr fieldInfo;
		
		DocFieldProcessorPerFieldPtr next;
		int32_t lastGen;
		
		int32_t fieldCount;
		Collection<FieldablePtr> fields;
	
	public:
		virtual void abort();
	};
}
