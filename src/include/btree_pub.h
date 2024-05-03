#ifndef BTREE_PUB_H
#define BTREE_PUB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "algo_typde_def.h"

typedef void * BTreeHandle;
typedef struct BTreeNode BTreeNodeT;

typedef struct BTreeScanCursor BTreeScanCursorT;

int BTreeLookUp(BTreeHandle handle, uint32_t key, uint8_t **value);

void BTreeDelete(BTreeHandle handle, uint32_t key);

void BTreeInsert(BTreeHandle handle, uint32_t key, uint8_t *value, uint32_t valueLen);

void BTreeCreate(int m, BTreeHandle *handle);

void BTreeDrop(BTreeHandle handle);

void BTreeScanBegin(BTreeHandle handle, int keyBegin, int keyEnd, BTreeScanCursorT **cursor);

bool BTreeScanFetch(BTreeScanCursorT *cursor, uint32_t *key, uint8_t **value, uint32_t *valueLen);

bool BTreeScanMove(BTreeScanCursorT *cursor);

void BTreeScanEnd(BTreeScanCursorT *cursor);

#ifdef __cplusplus
}
#endif

#endif