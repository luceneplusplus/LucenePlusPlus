/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "NormsWriterPerField.h"
#include "NormsWriterPerThread.h"
#include "Similarity.h"
#include "DocInverterPerField.h"
#include "DocumentsWriter.h"
#include "_DocumentsWriter.h"
#include "FieldInfo.h"
#include "MiscUtils.h"

namespace Lucene
{
    NormsWriterPerField::NormsWriterPerField(DocInverterPerFieldPtr docInverterPerField, NormsWriterPerThreadPtr perThread, FieldInfoPtr fieldInfo)
    {
        docIDs = Collection<int32_t>::newInstance(1);
        norms = ByteArray::newInstance(1);
        upto = 0;
        
        this->_perThread = perThread;
        this->fieldInfo = fieldInfo;
        docState = perThread->docState;
        fieldState = docInverterPerField->fieldState;
    }
    
    NormsWriterPerField::~NormsWriterPerField()
    {
    }
    
    void NormsWriterPerField::reset()
    {
        // Shrink back if we are over allocated now
        MiscUtils::shrink(docIDs, upto);
        MiscUtils::shrink(norms, upto);
        upto = 0;
    }
    
    void NormsWriterPerField::abort()
    {
        upto = 0;
    }
    
    int32_t NormsWriterPerField::compareTo(LuceneObjectPtr other)
    {
        return fieldInfo->name.compare(boost::static_pointer_cast<NormsWriterPerField>(other)->fieldInfo->name);
    }
    
    void NormsWriterPerField::finish()
    {
        if (fieldInfo->isIndexed && !fieldInfo->omitNorms)
        {
            if (docIDs.size() <= upto)
            {
                BOOST_ASSERT(docIDs.size() == upto);
                MiscUtils::grow(docIDs, 1 + upto);
            }
            if (norms.size() <= upto)
            {
                BOOST_ASSERT(norms.size() == upto);
                MiscUtils::grow(norms, 1 + upto);
            }
            double norm = docState->similarity->computeNorm(fieldInfo->name, fieldState);
            norms[upto] = docState->similarity->encodeNormValue(norm);
            docIDs[upto] = docState->docID;
            ++upto;
        }
    }
}
