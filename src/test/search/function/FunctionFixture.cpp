/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "FunctionFixture.h"
#include "RAMDirectory.h"
#include "StandardAnalyzer.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "VariantUtils.h"

namespace Lucene {

/// Actual score computation order is slightly different than assumptions this allows for a small amount of variation
const double FunctionFixture::TEST_SCORE_TOLERANCE_DELTA = 0.001;

const int32_t FunctionFixture::N_DOCS = 17;

const String FunctionFixture::ID_FIELD = L"id";
const String FunctionFixture::TEXT_FIELD = L"text";
const String FunctionFixture::INT_FIELD = L"iii";
const String FunctionFixture::DOUBLE_FIELD = L"fff";

FunctionFixture::FunctionFixture(bool doMultiSegment) {
    this->doMultiSegment = doMultiSegment;

    // prepare a small index with just a few documents.
    dir = newLucene<RAMDirectory>();
    anlzr = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    IndexWriterPtr iw = newLucene<IndexWriter>(dir, anlzr, IndexWriter::MaxFieldLengthLIMITED);
    // add docs not exactly in natural ID order, to verify we do check the order of docs by scores
    int32_t remaining = N_DOCS;
    Collection<uint8_t> done = Collection<uint8_t>::newInstance(N_DOCS);
    int32_t i = 0;
    while (remaining > 0) {
        if (done[i]) {
            boost::throw_exception(RuntimeException(L"to set this test correctly N_DOCS=" + StringUtils::toString(N_DOCS) + L" must be primary and greater than 2!"));
        }
        addDoc(iw, i);
        done[i] = true;
        i = (i + 4) % N_DOCS;
        if (doMultiSegment && remaining % 3 == 0) {
            iw->commit();
        }
        --remaining;
    }
    iw->close();
}

FunctionFixture::~FunctionFixture() {
}

const Collection<String> FunctionFixture::DOC_TEXT_LINES() {
    static Collection<String> _DOC_TEXT_LINES;
    if (!_DOC_TEXT_LINES) {
        _DOC_TEXT_LINES = Collection<String>::newInstance();
        _DOC_TEXT_LINES.add(L"Well, this is just some plain text we use for creating the ");
        _DOC_TEXT_LINES.add(L"test documents. It used to be a text from an online collection ");
        _DOC_TEXT_LINES.add(L"devoted to first aid, but if there was there an (online) lawyers ");
        _DOC_TEXT_LINES.add(L"first aid collection with legal advices, \"it\" might have quite ");
        _DOC_TEXT_LINES.add(L"probably advised one not to include \"it\"'s text or the text of ");
        _DOC_TEXT_LINES.add(L"any other online collection in one's code, unless one has money ");
        _DOC_TEXT_LINES.add(L"that one don't need and one is happy to donate for lawyers ");
        _DOC_TEXT_LINES.add(L"charity. Anyhow at some point, rechecking the usage of this text, ");
        _DOC_TEXT_LINES.add(L"it became uncertain that this text is free to use, because ");
        _DOC_TEXT_LINES.add(L"the web site in the disclaimer of he eBook containing that text ");
        _DOC_TEXT_LINES.add(L"was not responding anymore, and at the same time, in projGut, ");
        _DOC_TEXT_LINES.add(L"searching for first aid no longer found that eBook as well. ");
        _DOC_TEXT_LINES.add(L"So here we are, with a perhaps much less interesting ");
        _DOC_TEXT_LINES.add(L"text for the test, but oh much much safer. ");
    }
    return _DOC_TEXT_LINES;
}

void FunctionFixture::addDoc(const IndexWriterPtr& iw, int32_t i) {
    DocumentPtr d = newLucene<Document>();
    int32_t scoreAndID = i + 1;

    FieldPtr f = newLucene<Field>(ID_FIELD, id2String(scoreAndID), Field::STORE_YES, Field::INDEX_NOT_ANALYZED); // for debug purposes
    f->setOmitNorms(true);
    d->add(f);

    f = newLucene<Field>(TEXT_FIELD, L"text of doc" + StringUtils::toString(scoreAndID) + textLine(i), Field::STORE_NO, Field::INDEX_ANALYZED); // for regular search
    f->setOmitNorms(true);
    d->add(f);

    f = newLucene<Field>(INT_FIELD, StringUtils::toString(scoreAndID), Field::STORE_YES, Field::INDEX_NOT_ANALYZED); // for function scoring
    f->setOmitNorms(true);
    d->add(f);

    f = newLucene<Field>(DOUBLE_FIELD, StringUtils::toString(scoreAndID) + L".000", Field::STORE_YES, Field::INDEX_NOT_ANALYZED); // for function scoring
    f->setOmitNorms(true);
    d->add(f);

    iw->addDocument(d);
}

String FunctionFixture::id2String(int32_t scoreAndID) {
    String s = L"000000000" + StringUtils::toString(scoreAndID); // 17 --> ID00017
    int32_t n = StringUtils::toString(N_DOCS).length() + 3;
    int32_t k = s.length() - n;
    return L"ID" + s.substr(k);
}

String FunctionFixture::textLine(int32_t docNum) {
    // some text line for regular search
    return DOC_TEXT_LINES()[docNum % DOC_TEXT_LINES().size()];
}

double FunctionFixture::expectedFieldScore(const String& docIDFieldVal) {
    // extract expected doc score from its ID Field: "ID7" --> 7.0
    return StringUtils::toDouble(docIDFieldVal.substr(2));
}

bool FunctionFixture::equalCollectionValues(CollectionValue first, CollectionValue second) {
    if (!VariantUtils::equalsType(first, second)) {
        return false;
    }
    if (VariantUtils::typeOf< Collection<uint8_t> >(first)) {
        return (VariantUtils::get< Collection<uint8_t> >(first).hashCode() == VariantUtils::get< Collection<uint8_t> >(second).hashCode());
    }
    if (VariantUtils::typeOf< Collection<int32_t> >(first)) {
        return (VariantUtils::get< Collection<int32_t> >(first).hashCode() == VariantUtils::get< Collection<int32_t> >(second).hashCode());
    }
    if (VariantUtils::typeOf< Collection<double> >(first)) {
        return (VariantUtils::get< Collection<double> >(first).hashCode() == VariantUtils::get< Collection<double> >(second).hashCode());
    }
    return false;
}

}
