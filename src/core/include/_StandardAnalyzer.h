/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _STANDARDANALYZER_H
#define _STANDARDANALYZER_H

#include "ReusableAnalyzerBase.h"

namespace Lucene
{
    class StandardAnalyzerTokenStreamComponents : public TokenStreamComponents
    {
    public:
        StandardAnalyzerTokenStreamComponents(StandardAnalyzerPtr analyzer, TokenizerPtr source, TokenStreamPtr result);
        StandardAnalyzerTokenStreamComponents(StandardAnalyzerPtr analyzer, TokenizerPtr source);
        
        virtual ~StandardAnalyzerTokenStreamComponents();
        
        LUCENE_CLASS(StandardAnalyzerTokenStreamComponents);
    
    protected:
        StandardAnalyzerWeakPtr _analyzer;
    
    protected:
        virtual bool reset(ReaderPtr reader);
        
    };
}

#endif
