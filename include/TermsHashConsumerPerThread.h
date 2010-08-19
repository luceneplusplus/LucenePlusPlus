/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI TermsHashConsumerPerThread : public LuceneObject
	{
	public:
		virtual ~TermsHashConsumerPerThread();
		
		LUCENE_CLASS(TermsHashConsumerPerThread);
	
	public:
		virtual void startDocument() = 0;
		virtual DocWriterPtr finishDocument() = 0;
		virtual TermsHashConsumerPerFieldPtr addField(TermsHashPerFieldPtr termsHashPerField, FieldInfoPtr fieldInfo) = 0;
		virtual void abort() = 0;
	};
}
