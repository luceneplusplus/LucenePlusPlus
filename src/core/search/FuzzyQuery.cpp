/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "FuzzyQuery.h"
#include "FuzzyTermEnum.h"
#include "SingleTermEnum.h"
#include "Term.h"
#include "TermQuery.h"
#include "BooleanQuery.h"
#include "BooleanClause.h"
#include "TopTermsRewrite.h"
#include "MiscUtils.h"

namespace Lucene
{
    const int32_t FuzzyQuery::defaultPrefixLength = 0;
    
    const int32_t FuzzyQuery::defaultMaxExpansions = INT_MAX;
    
    FuzzyQuery::FuzzyQuery(TermPtr term, double minimumSimilarity, int32_t prefixLength, int32_t maxExpansions)
    {
        ConstructQuery(term, minimumSimilarity, prefixLength, maxExpansions);
    }
    
    FuzzyQuery::FuzzyQuery(TermPtr term, double minimumSimilarity, int32_t prefixLength)
    {
        ConstructQuery(term, minimumSimilarity, prefixLength, defaultMaxExpansions);
    }
    
    FuzzyQuery::FuzzyQuery(TermPtr term, double minimumSimilarity)
    {
        ConstructQuery(term, minimumSimilarity, defaultPrefixLength, defaultMaxExpansions);
    }
    
    FuzzyQuery::FuzzyQuery(TermPtr term)
    {
        ConstructQuery(term, defaultMinSimilarity(), defaultPrefixLength, defaultMaxExpansions);
    }
    
    FuzzyQuery::~FuzzyQuery()
    {
    }
    
    void FuzzyQuery::ConstructQuery(TermPtr term, double minimumSimilarity, int32_t prefixLength, int32_t maxExpansions)
    {
        this->term = term;
        
        if (minimumSimilarity >= 1.0)
            boost::throw_exception(IllegalArgumentException(L"minimumSimilarity >= 1"));
        else if (minimumSimilarity < 0.0)
            boost::throw_exception(IllegalArgumentException(L"minimumSimilarity < 0"));
        if (prefixLength < 0)
            boost::throw_exception(IllegalArgumentException(L"prefixLength < 0"));
        if (maxExpansions < 0)
            boost::throw_exception(IllegalArgumentException(L"maxExpansions < 0"));
        
        setRewriteMethod(newLucene<TopTermsScoringBooleanQueryRewrite>(maxExpansions));
        
        this->termLongEnough = ((int32_t)term->text().length() > (int32_t)(1.0 / (1.0 - minimumSimilarity)));
        
        this->minimumSimilarity = minimumSimilarity;
        this->prefixLength = prefixLength;
        rewriteMethod = SCORING_BOOLEAN_QUERY_REWRITE();
    }
    
    double FuzzyQuery::defaultMinSimilarity()
    {
        const double _defaultMinSimilarity = 0.5;
        return _defaultMinSimilarity;
    }
    
    double FuzzyQuery::getMinSimilarity()
    {
        return minimumSimilarity;
    }
    
    int32_t FuzzyQuery::getPrefixLength()
    {
        return prefixLength;
    }
    
    FilteredTermEnumPtr FuzzyQuery::getEnum(IndexReaderPtr reader)
    {
        if (!termLongEnough) // can only match if it's exact
            return newLucene<SingleTermEnum>(reader, term);
        return newLucene<FuzzyTermEnum>(reader, getTerm(), minimumSimilarity, prefixLength);
    }
    
    TermPtr FuzzyQuery::getTerm()
    {
        return term;
    }
    
    String FuzzyQuery::getField()
    {
        return term->field();
    }
    
    LuceneObjectPtr FuzzyQuery::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = MultiTermQuery::clone(other ? other : newLucene<FuzzyQuery>(term));
        FuzzyQueryPtr cloneQuery(boost::static_pointer_cast<FuzzyQuery>(clone));
        cloneQuery->minimumSimilarity = minimumSimilarity;
        cloneQuery->prefixLength = prefixLength;
        cloneQuery->termLongEnough = termLongEnough;
        cloneQuery->term = term;
        return cloneQuery;
    }
    
    String FuzzyQuery::toString(const String& field)
    {
        StringStream buffer;
        if (term->field() != field)
            buffer << term->field() << L":";
        buffer << term->text() << L"~" << minimumSimilarity << boostString();
        return buffer.str();
    }
    
    int32_t FuzzyQuery::hashCode()
    {
        int32_t prime = 31;
        int32_t result = MultiTermQuery::hashCode();
        result = prime * result + MiscUtils::doubleToIntBits(minimumSimilarity);
        result = prime * result + prefixLength;
        result = prime * result + (term ? term->hashCode() : 0);
        return result;
    }
    
    bool FuzzyQuery::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        if (!MultiTermQuery::equals(other))
            return false;
        if (!MiscUtils::equalTypes(shared_from_this(), other))
            return false;
        FuzzyQueryPtr otherFuzzyQuery(boost::dynamic_pointer_cast<FuzzyQuery>(other));
        if (!otherFuzzyQuery)
            return false;
        if (MiscUtils::doubleToIntBits(minimumSimilarity) != MiscUtils::doubleToIntBits(otherFuzzyQuery->minimumSimilarity))
            return false;
        if (prefixLength != otherFuzzyQuery->prefixLength)
            return false;
        if (!term)
        {
            if (otherFuzzyQuery->term)
                return false;
        }
        else if (!term->equals(otherFuzzyQuery->term))
            return false;
        return true;
    }
}
