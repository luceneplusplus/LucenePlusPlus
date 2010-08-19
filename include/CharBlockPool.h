/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI CharBlockPool : public LuceneObject
	{
	public:
		CharBlockPool(DocumentsWriterPtr docWriter);
		virtual ~CharBlockPool();
		
		LUCENE_CLASS(CharBlockPool);
	
	public:
		Collection<CharArray> buffers;
		int32_t numBuffer;
		int32_t bufferUpto; // Which buffer we are up to
		int32_t charUpto; // Where we are in head buffer
		
		CharArray buffer; // Current head buffer
		int32_t charOffset; // Current head offset
	
	protected:
		DocumentsWriterWeakPtr _docWriter;
	
	public:
		void reset();
		void nextBuffer();
	};
}
