#ifndef __AHR_MAP_H__
#define __AHR_MAP_H__

#include <stdlib.h>

typedef struct
{
    void *key;
    void *data;
} AHR_MapPair_t;

typedef struct
{
    AHR_MapPair_t *data;
    size_t max_size;
} AHR_Map_t;



#endif
