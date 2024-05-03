/*
B+树的性质
1> 节点子女个数 [(m + 1)/2, m]
*/
#ifndef BTREE_H
#define BTREE_H
#include "btree_pub.h"

typedef struct BTreeSlot {
    int key;
    union {
        struct BTreeNode *child;
        struct {
            void *value;
            int valueLen;
        };
    };
} BTreeSlotT;

struct BTreeNode {
    uint32_t id; // 用于debug
    int keyNum;
    int level;
    struct BTreeNode *next;
    struct BTreeNode *prev;
    BTreeSlotT slot[0];
};

typedef struct BTreeDesc {
    int m; // m阶B树，节点内关键字最大个数
    int min; // m 阶 B树，节点内关键字最小个数
    int valueLen;
    BTreeNodeT *root;
    BTreeNodeT *first;
    BTreeNodeT *last;
} BTreeDescT;

void BTreeInsert(BTreeHandle handle, uint32_t key, uint8_t *value, uint32_t valueLen);

struct BTreeScanCursor {
    BTreeDescT *desc;
    int keyBegin;
    int keyEnd;
    bool isEnd;
    int curKey;
    uint8_t *curValue;
    uint32_t curValueLen;
    BTreeNodeT *curNode;
    uint32_t curSlotIndex;
};



#endif