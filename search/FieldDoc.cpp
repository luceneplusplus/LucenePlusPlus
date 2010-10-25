#include "LuceneInc.h"
#include "FieldDoc.h"

namespace Lucene
{
    FieldDoc::FieldDoc(int32_t doc, double score, Collection<ComparableValue> fields) : ScoreDoc(doc, score)
    {
        this->fields = fields;
    }
    
    FieldDoc::~FieldDoc()
    {
    }
    
    String FieldDoc::toString()
    {
        StringStream buffer;
        buffer << ScoreDoc::toString() << L"[";
        for (Collection<ComparableValue>::iterator field = fields.begin(); field != fields.end(); ++field)
        {
            if (field != fields.begin())
                buffer << L", ";
            buffer << *field;
        }
        buffer << L"]";
        return buffer.str();
    }
}
