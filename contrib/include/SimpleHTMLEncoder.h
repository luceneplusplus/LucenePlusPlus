/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Encoder.h"

namespace Lucene
{
	/// Simple {@link Encoder} implementation to escape text for HTML output.
	class LPPAPI SimpleHTMLEncoder : public Encoder, public LuceneObject
	{
	public:
		virtual ~SimpleHTMLEncoder();
		LUCENE_CLASS(SimpleHTMLEncoder);
	
	public:
		virtual String encodeText(const String& originalText);
		
		/// Encode string into HTML
		static String htmlEncode(const String& plainText);
	};
}
