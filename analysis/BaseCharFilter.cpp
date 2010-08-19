/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseCharFilter.h"

namespace Lucene
{
    BaseCharFilter::BaseCharFilter(CharStreamPtr in) : CharFilter(in)
    {
    }
    
    BaseCharFilter::~BaseCharFilter()
    {
    }
    
    int32_t BaseCharFilter::correct(int32_t currentOff)
    {
        if (!pcmList || pcmList.empty())
            return currentOff;
        for (int32_t i = pcmList.size() - 1; i >= 0; --i)
        {
            if (currentOff >=  pcmList[i]->off)
                return currentOff + pcmList[i]->cumulativeDiff;
        }
        return currentOff;
    }
    
    int32_t BaseCharFilter::getLastCumulativeDiff()
    {
        return (!pcmList || pcmList.empty()) ? 0 : pcmList[pcmList.size() - 1]->cumulativeDiff;
    }
    
    void BaseCharFilter::addOffCorrectMap(int32_t off, int32_t cumulativeDiff)
    {
        if (!pcmList)
            pcmList = Collection<OffCorrectMapPtr>::newInstance();
        pcmList.add(newLucene<OffCorrectMap>(off, cumulativeDiff));
    }
    
    OffCorrectMap::OffCorrectMap(int32_t off, int32_t cumulativeDiff)
    {
        this->off = off;
        this->cumulativeDiff = cumulativeDiff;
    }
    
    OffCorrectMap::~OffCorrectMap()
    {
    }
    
    String OffCorrectMap::toString()
    {
        StringStream buffer;
        buffer << L"(" << off << L"," << cumulativeDiff << L")";
        return buffer.str();
    }
}
