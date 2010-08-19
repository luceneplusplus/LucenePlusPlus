/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI InvertedDocConsumerPerThread : public LuceneObject
	{
	public:
		virtual ~InvertedDocConsumerPerThread();
		
		LUCENE_CLASS(InvertedDocConsumerPerThread);
	
	public:
		virtual void startDocument() = 0;
		virtual InvertedDocConsumerPerFieldPtr addField(DocInverterPerFieldPtr docInverterPerField, FieldInfoPtr fieldInfo) = 0;
		virtual DocWriterPtr finishDocument() = 0;
		virtual void abort() = 0;
	};
}
