/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef NORMSWRITERPERTHREAD_H
#define NORMSWRITERPERTHREAD_H

#include "InvertedDocEndConsumerPerThread.h"

namespace Lucene
{
    class NormsWriterPerThread : public InvertedDocEndConsumerPerThread
    {
    public:
        NormsWriterPerThread(DocInverterPerThreadPtr docInverterPerThread, NormsWriterPtr normsWriter);
        virtual ~NormsWriterPerThread();

        LUCENE_CLASS(NormsWriterPerThread);

    public:
        NormsWriterPtr normsWriter;
        DocStatePtr docState;

    public:
        virtual InvertedDocEndConsumerPerFieldPtr addField(DocInverterPerFieldPtr docInverterPerField, FieldInfoPtr fieldInfo);
        virtual void abort();
        virtual void startDocument();
        virtual void finishDocument();

        bool freeRAM();
    };
}

#endif
