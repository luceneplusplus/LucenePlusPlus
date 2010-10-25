/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FORMATPOSTINGSFIELDSWRITER_H
#define FORMATPOSTINGSFIELDSWRITER_H

#include "FormatPostingsFieldsConsumer.h"

namespace Lucene
{
    class LPPAPI FormatPostingsFieldsWriter : public FormatPostingsFieldsConsumer
    {
    public:
        FormatPostingsFieldsWriter(SegmentWriteStatePtr state, FieldInfosPtr fieldInfos);
        virtual ~FormatPostingsFieldsWriter();
        
        LUCENE_CLASS(FormatPostingsFieldsWriter);
            
    public:
        DirectoryPtr dir;
        String segment;
        TermInfosWriterPtr termsOut;
        SegmentWriteStatePtr state;
        FieldInfosPtr fieldInfos;
        FormatPostingsTermsWriterPtr termsWriter;
        DefaultSkipListWriterPtr skipListWriter;
        int32_t totalNumDocs;
    
    public:
        virtual void initialize();
        
        /// Add a new field.
        virtual FormatPostingsTermsConsumerPtr addField(FieldInfoPtr field);
        
        /// Called when we are done adding everything.
        virtual void finish();
    };
}

#endif
