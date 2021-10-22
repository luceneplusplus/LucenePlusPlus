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
struct index_document_t { DocumentPtr rep;};



index_t* index_open(const char *path) {
   IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(StringUtils::toString(path)), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
   if (writer == NULL) {return NULL;}
   index_t *index = new index_t;
   if (index == NULL) { return NULL; } 
   index->rep = writer;
   return index; 
}

int index_put(index_t *index, index_document_t *idoc) {
  index->rep->addDocument(idoc->rep);  
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
index_document_t* index_document_create() {
  DocumentPtr doc = newLucene<Document>();
  if (doc == NULL) { return NULL; }    
  index_document_t *idoc = new index_document_t;
  idoc->rep = doc;
  return idoc;
} 


void index_document_destroy(index_document_t *idoc) {
  if (idoc == NULL) { return; }   
  idoc->rep = NULL; 
  delete idoc; 
}
void index_document_add(index_document_t *idoc, const char *field, int nFields, const char *val, int nVals, int32_t index) {
  if (index) {
    idoc->rep->add(newLucene<Field>(StringUtils::toString(field),  StringUtils::toString(val), Field::STORE_YES, Field::INDEX_NOT_ANALYZED_NO_NORMS)); 
  } else {
    idoc->rep->add(newLucene<Field>(UID, StringUtils::toString(val), Field::STORE_YES, Field::INDEX_NO));
  }
}
}
