/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Formatter.h"

namespace Lucene
{
	/// Simple {@link Formatter} implementation to highlight terms with a pre and post tag.
	class LPPAPI SimpleHTMLFormatter : public Formatter, public LuceneObject
	{
	public:
	    /// Default constructor uses HTML: &lt;B&gt; tags to markup terms.
	    SimpleHTMLFormatter();	    
	    
	    SimpleHTMLFormatter(const String& preTag, const String& postTag);
	    
		virtual ~SimpleHTMLFormatter();
		
		LUCENE_CLASS(SimpleHTMLFormatter);
	
	protected:
	    static const String DEFAULT_PRE_TAG;
	    static const String DEFAULT_POST_TAG;
	    
	    String preTag;
	    String postTag;
	
	public:
		virtual String highlightTerm(const String& originalText, TokenGroupPtr tokenGroup);
	};
}
