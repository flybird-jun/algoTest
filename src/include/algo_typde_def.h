#ifndef ALGO_TYPDE_DEF_H
#define ALGO_TYPDE_DEF_H
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
typedef unsigned char bool;
#define TRUE 1
#define FALSE 0
#define true 1
#define false 0
#endif

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

static inline void *AlgoMalloc(uint32_t size)
{
    void *ptr = malloc(size);
    return ptr;
}

#ifdef __cplusplus
}
#endif
#endif