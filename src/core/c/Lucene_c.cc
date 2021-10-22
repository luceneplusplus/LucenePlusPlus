#include "Lucene_c.h"


#include "targetver.h"
#include <iostream>
#include "LuceneHeaders.h"
#include "FileUtils.h"
#include "MiscUtils.h"
#include "ConstantScoreQuery.h"

using namespace Lucene;
String UID = L"U$DID";
extern "C" {

struct index_t { IndexWriterPtr rep; };

index_t* index_open(const char *path) {
   IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(StringUtils::toString(path)), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
   if (writer == NULL) {return NULL;}
   index_t *index = new index_t;
   if (index == NULL) { return NULL; } 
   index->rep = writer;
   return index; 
}

int index_put(index_t *index, const char **field, int32_t nField, const char **key, int32_t nKey, int32_t uid) {
  DocumentPtr doc = newLucene<Document>();
  for (int i = 0; i < nField; i++) {
    doc->add(newLucene<Field>(StringUtils::toString((*field)[i]),  StringUtils::toString((*key)[i]), Field::STORE_YES, Field::INDEX_NOT_ANALYZED_NO_NORMS)); 
  }
  doc->add(newLucene<Field>(UID, StringUtils::toString(uid), Field::STORE_YES, Field::INDEX_NO));
  index->rep->addDocument(doc);  
  return 1;
}

int index_mulit_search(index_t *index, const char **field, int32_t nField, const char **key, int32_t nKey, int type, int **result, int32_t *nResult) {
  if(type == 0) {
  } else if (type == 1) {
    PrefixFilterPtr filter = newLucene<PrefixFilter>(newLucene<Term>(StringUtils::toString((*field)), StringUtils::toString((*key))));    
    QueryPtr query = newLucene<ConstantScoreQuery>(filter); 
    IndexReaderPtr reader = index->rep->getReader() ; 
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);  
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs; 
    if (*nResult < hits.size()) {
      *result = (int *)realloc(*result, hits.size() * sizeof(int));
      *nResult = hits.size();
    }
    for (int i = 0; i < hits.size(); i++) {
      (*result)[i] = StringUtils::toInt(searcher->doc(hits[i]->doc)->get(UID)); 
    }
  } else if (type == 2) {

  } else if (type == 3);
  return 1;
}
int index_search(index_t *index, const char *field, int32_t nField, const char *key, int32_t nKey, int type, int **result, int32_t *nResult) {
  if(type == 0) {
  } else if (type == 1) {
    PrefixFilterPtr filter = newLucene<PrefixFilter>(newLucene<Term>(StringUtils::toString((field)), StringUtils::toString((key))));    
    QueryPtr query = newLucene<ConstantScoreQuery>(filter); 
    IndexReaderPtr reader = index->rep->getReader() ; 
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);  
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs; 
    if (*nResult < hits.size()) {
      *result = (int *)realloc(*result, hits.size() * sizeof(int));
      *nResult = hits.size();
    }
    for (int i = 0; i < hits.size(); i++) {
      (*result)[i] = StringUtils::toInt(searcher->doc(hits[i]->doc)->get(UID)); 
    }
  } else if (type == 2) {

  } else if (type == 3);
  return 1;
}

void index_close(index_t *index) {
  if (index->rep) {
    index->rep->close();
    index->rep = NULL;
  } 
  delete index;
}

int index_optimize(index_t *index) {
  index->rep->optimize();
  return 1;
}


}
