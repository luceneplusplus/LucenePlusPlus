#ifndef _LUCENE_C_H
#define _LUCENE_C_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>


typedef struct index_t index_t;

__attribute__((visibility("default"))) index_t* index_open(const char *path);


__attribute__((visibility("default"))) int index_put(index_t *index, const char **field, int32_t nField, const char **key, int32_t nKey, int32_t uid);

__attribute__((visibility("default"))) int index_search(index_t *index, const char **field, int32_t nField, const char **key, int32_t nKey, int type, int **result, int32_t *nResult);

__attribute__((visibility("default"))) void index_close(index_t *index); 

__attribute__((visibility("default"))) int index_optimize(index_t *index);


#ifdef __cplusplus
}
#endif

#endif
