/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GREEKSTEMMER_H
#define GREEKSTEMMER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene
{
    /// A stemmer for Greek words, according to: Development of a Stemmer for the
    /// Greek Language. Georgios Ntais
    ///
    /// NOTE: Input is expected to be casefolded for Greek (including folding of
    /// final sigma to sigma), and with diacritics removed. This can be achieved
    /// with {@link GreekLowerCaseFilter}.
    class LPPCONTRIBAPI GreekStemmer : public LuceneObject
    {
    public:
        GreekStemmer();
        virtual ~GreekStemmer();

        LUCENE_CLASS(GreekStemmer);

    public:
        int32_t stem(wchar_t* s, int32_t len);

    private:
        int32_t rule0(wchar_t* s, int32_t len);
        int32_t rule1(wchar_t* s, int32_t len);
        int32_t rule2(wchar_t* s, int32_t len);
        int32_t rule3(wchar_t* s, int32_t len);
        int32_t rule4(wchar_t* s, int32_t len);
        int32_t rule5(wchar_t* s, int32_t len);
        int32_t rule6(wchar_t* s, int32_t len);
        int32_t rule7(wchar_t* s, int32_t len);
        int32_t rule8(wchar_t* s, int32_t len);
        int32_t rule9(wchar_t* s, int32_t len);
        int32_t rule10(wchar_t* s, int32_t len);
        int32_t rule11(wchar_t* s, int32_t len);
        int32_t rule12(wchar_t* s, int32_t len);
        int32_t rule13(wchar_t* s, int32_t len);
        int32_t rule14(wchar_t* s, int32_t len);
        int32_t rule15(wchar_t* s, int32_t len);
        int32_t rule16(wchar_t* s, int32_t len);
        int32_t rule17(wchar_t* s, int32_t len);
        int32_t rule18(wchar_t* s, int32_t len);
        int32_t rule19(wchar_t* s, int32_t len);
        int32_t rule20(wchar_t* s, int32_t len);
        int32_t rule21(wchar_t* s, int32_t len);
        int32_t rule22(wchar_t* s, int32_t len);

        int32_t endsWith(wchar_t* s, int32_t len, const String& suffix);
        int32_t endsWithVowel(wchar_t* s, int32_t len);
        int32_t endsWithVowelNoY(wchar_t* s, int32_t len);
    };
}

#endif

