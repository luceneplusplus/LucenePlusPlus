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
	/// Low-level class used to record information about a section of a document with a score.
	class LPPAPI TextFragment : public LuceneObject
	{
	public:
	    TextFragment(const String& markedUpText, int32_t textStartPos, int32_t fragNum);
		virtual ~TextFragment();
		
		LUCENE_CLASS(TextFragment);
	
	public:
        String markedUpText;
        int32_t fragNum;
        int32_t textStartPos;
        int32_t textEndPos;
        double score;
    
    public:
        void setScore(double score);
        double getScore();
        
        /// @param frag2 Fragment to be merged into this one
        void merge(TextFragmentPtr frag2);
        
        /// @return true if this fragment follows the one passed
        bool follows(TextFragmentPtr fragment);
        
        /// @return the fragment sequence number
        int32_t getFragNum();
        
        /// Returns the marked-up text for this text fragment
        virtual String toString();
	};
}
