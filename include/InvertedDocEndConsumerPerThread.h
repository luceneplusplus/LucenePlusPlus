/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI InvertedDocEndConsumerPerThread : public LuceneObject
	{
	public:
		virtual ~InvertedDocEndConsumerPerThread();
		
		LUCENE_CLASS(InvertedDocEndConsumerPerThread);
	
	public:
		virtual void startDocument() = 0;
		virtual InvertedDocEndConsumerPerFieldPtr addField(DocInverterPerFieldPtr docInverterPerField, FieldInfoPtr fieldInfo) = 0;
		virtual void finishDocument() = 0;
		virtual void abort() = 0;
	};
}
