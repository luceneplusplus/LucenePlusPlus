/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _CLASSICANALYZER_H
#define _CLASSICANALYZER_H

#include "ReusableAnalyzerBase.h"

namespace Lucene
{
    class ClassicAnalyzerTokenStreamComponents : public TokenStreamComponents
    {
    public:
        ClassicAnalyzerTokenStreamComponents(ClassicAnalyzerPtr analyzer, TokenizerPtr source, TokenStreamPtr result);
        ClassicAnalyzerTokenStreamComponents(ClassicAnalyzerPtr analyzer, TokenizerPtr source);
        
        virtual ~ClassicAnalyzerTokenStreamComponents();
        
        LUCENE_CLASS(ClassicAnalyzerTokenStreamComponents);
    
    protected:
        ClassicAnalyzerWeakPtr _analyzer;
    
    protected:
        virtual bool reset(ReaderPtr reader);
        
    };
}

#endif
