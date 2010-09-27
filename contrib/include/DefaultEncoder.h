/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Encoder.h"

namespace Lucene
{
	/// Simple {@link Encoder} implementation that does not modify the output.
	class LPPAPI DefaultEncoder : public Encoder, public LuceneObject
	{
	public:
		virtual ~DefaultEncoder();
		LUCENE_CLASS(DefaultEncoder);
	
	public:
		virtual String encodeText(const String& originalText);
	};
}
