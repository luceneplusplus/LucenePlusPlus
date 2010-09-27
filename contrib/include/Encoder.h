/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene
{
	/// Encodes original text. The Encoder works with the {@link Formatter} to generate output.
	class LPPAPI Encoder
	{
	public:
		virtual ~Encoder();
		LUCENE_INTERFACE(Encoder);
	
	public:
		virtual String encodeText(const String& originalText);
	};
}
