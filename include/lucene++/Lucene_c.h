#ifndef _LUCENE_C_H
#define _LUCENE_C_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>


typedef struct index_t index_t;
typedef struct index_document_t index_document_t;
__attribute__((visibility("default"))) index_t* index_open(const char *path);


__attribute__((visibility("default"))) int index_put(index_t *index, index_document_t *idoc);

__attribute__((visibility("default"))) int index_search(index_t *index, const char *field, int32_t nField, const char *key, int32_t nKey, int type, int **result, int32_t *nResult);

__attribute__((visibility("default"))) int index_multi_search(index_t *index, const char **field, int32_t nField, const char **key, int32_t nKey, int type, int **result, int32_t *nResult);

__attribute__((visibility("default"))) void index_close(index_t *index); 

__attribute__((visibility("default"))) int index_optimize(index_t *index);


__attribute__((visibility("default"))) index_document_t* index_document_create();

__attribute__((visibility("default"))) void index_document_add(index_document_t *idoc, const char **field, int nFields, const char **val, int nVals, int32_t uid); 

__attribute__((visibility("default"))) void index_document_destroy(index_document_t *doc);
#ifdef __cplusplus
}
#endif

#endif
