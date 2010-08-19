/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI InvertedDocEndConsumerPerField : public LuceneObject
	{
	public:
		virtual ~InvertedDocEndConsumerPerField();
		
		LUCENE_CLASS(InvertedDocEndConsumerPerField);
	
	public:
		virtual void finish() = 0;
		virtual void abort() = 0;
	};
}
