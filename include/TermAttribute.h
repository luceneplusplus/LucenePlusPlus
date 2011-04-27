/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TERMATTRIBUTE_H
#define TERMATTRIBUTE_H

#include "CharTermAttribute.h"

namespace Lucene
{
    /// The term text of a Token.
    /// @deprecated This class is not used anymore. The backwards layer in AttributeFactory uses 
    /// the replacement implementation.
    class LPPAPI TermAttribute : public CharTermAttribute
    {
    public:
        virtual ~TermAttribute();
        LUCENE_CLASS(TermAttribute);
    };
}

#endif
