#include <stdlib.h>
#include <string.h>
#include "btree_pub.h"
#include "btree.h"
#include "log.h"
// 创建B树时，返回一个根节点地址
void BTreeCreate(int m, BTreeHandle *handle)
{
    LogInit();
    BTreeDescT *desc = (BTreeDescT *)AlgoMalloc(sizeof(BTreeDescT));
    desc->m = m;
    desc->min = (m + 1) / 2;
    desc->root = NULL;
    desc->first = NULL;
    desc->last = NULL;
    *handle =(BTreeHandle)desc;
    LOG_PUT("create btree success");
}

BTreeNodeT *BTreeNodeMalloc(uint32_t m)
{
    /* 
        多申请一个slot的原因：如果m为奇数，没有多申请slot，插入key时，节点分裂时，会导致两个节点的关键字个数分别为m/2, (m+1)/2，如果关键字插入到分配关键字(（m+1）/2 )多的节点，
        会导致分配关键字少的节点不满足B+树性质关键字个数m/2 < (m+1)/2，如果想满足B+树性质，就必须提前算好要插入的关键字是在哪边。
        而多申请一个slot就可以先插入到节点中再进行分裂，这时两个节点都必然满足B+树性质，减少计算
    */
    BTreeNodeT *p = (BTreeNodeT *)AlgoMalloc(sizeof(BTreeNodeT) + sizeof(BTreeSlotT) * (m + 1));
    p->prev = p;
    p->next = p;
    p->level = 0;
    p->keyNum = 0;
    static uint32_t id = 0;
    p->id = id++;
    return p;
}

// 向节点插入（append）数据，调用者必须保证节点可以插入，否则会assert
void BTreeNodeInsertSlot(BTreeNodeT *node, int index, BTreeSlotT *slot, bool isCopy)
{

    for (uint32_t i = node->keyNum; i > index; i--) {
        node->slot[i] = node->slot[i-1];
    }
    node->slot[index] = *slot;
    LOG_PUT("node id = %u, level = %u, index = %u, insert key = %u", node->id, node->level, index, node->slot[index].key);
    if (isCopy) {
        node->slot[index].value = AlgoMalloc(slot->valueLen);
        memcpy(node->slot[index].value, slot->value, slot->valueLen);
        LOG_PUT("key = %u, value = %p", slot->key, node->slot[index].value);
    }
    node->keyNum++;
}
// 插入新的叶子节点，更新叶子节点链表指针
void BTreeInsertNewLeafNode(BTreeNodeT *node, BTreeNodeT *newNode)
{
    newNode->next = node->next;
    node->next->prev = newNode;
    newNode->prev = node;
    node->next = newNode;
}

void BTreeNodeSplit(BTreeDescT *desc, BTreeNodeT *node, BTreeNodeT **pNewNode)
{
    uint32_t j = node->keyNum / 2;
    BTreeNodeT *newNode = BTreeNodeMalloc(desc->m);
    newNode->level = node->level;
    for (uint32_t i = j; i < node->keyNum; i++) {
        BTreeNodeInsertSlot(newNode, newNode->keyNum, &node->slot[i], false);
    }
    LOG_PUT("node split id = %u, keyNum = %u", node->id, j);
    node->keyNum = j; 
    *pNewNode = newNode;
    if (node->level == 0) {
        BTreeInsertNewLeafNode(node, newNode);
        if (desc->last == node) {
            desc->last = newNode;
        }
    }
}

bool BTreeNodeInsertMaxKey(BTreeDescT *desc, BTreeNodeT *node, BTreeSlotT *slot, BTreeNodeT **pNewNode)
{
    BTreeSlotT iSlot;
    BTreeSlotT *targetSlot = slot;
    bool isLeafNode;
    if (node->level != 0) {
        bool isSplit = BTreeNodeInsertMaxKey(desc, node->slot[node->keyNum - 1].child, slot, pNewNode);
        BTreeNodeT *leftChild = node->slot[node->keyNum - 1].child;
        // 当前节点的最大key改成对应孩子节点的最大值，如果子节点进行了分裂，再在当前节点插入maxKey
        LOG_PUT("id = %u, key = %u -> %u", node->id, 
            node->slot[node->keyNum - 1].key, 
            leftChild->slot[leftChild->keyNum - 1].key
        );
        node->slot[node->keyNum - 1].key = leftChild->slot[leftChild->keyNum - 1].key;
        
        if (!isSplit) {
            return false;
        }
        iSlot = (BTreeSlotT ) {
            .key = slot->key,
            .child = *pNewNode
        };
        targetSlot = &iSlot;
    }
    BTreeNodeInsertSlot(node, node->keyNum, targetSlot, node->level == 0);
    if (node->keyNum <= desc->m) {
        return false;
    }
    BTreeNodeSplit(desc, node, pNewNode);
    return true;
}

// 返回True 表示需要向父结点插入数据
// newNode 表示分裂后的新节点
bool BTreeNodeInsert(BTreeDescT *desc, BTreeNodeT *node, BTreeSlotT *slot, BTreeNodeT **pNewNode)
{
    BTreeSlotT iSlot;
    BTreeSlotT *targetSlot = slot;
    for (int i = 0; i < node->keyNum; i++) {
        if (slot->key <= node->slot[i].key){ 
            if (node->level != 0) {
                bool isSplit = BTreeNodeInsert(desc, node->slot[i].child, slot, pNewNode);
                if (!isSplit) { // 子节点没有分裂，可以直接返回
                    return false;
                }
                
                iSlot = (BTreeSlotT) { // 将分裂后的左边节点的最大值插入父节点
                        .key = node->slot[i].child->slot[node->slot[i].child->keyNum - 1].key,
                        .child = node->slot[i].child
                };
                node->slot[i].child = *pNewNode;
                targetSlot = &iSlot; 
            }
            BTreeNodeInsertSlot(node, i, targetSlot, node->level == 0);
            if (node->keyNum <= desc->m) { // 插入slot后仍然满足B+树性质，不需要分裂
                return false;
            }
            // 当前节点需要分裂
            BTreeNodeSplit(desc, node, pNewNode);
            return true;
        }
    }
    // for 循环没有返回说明是比当前最大值还大
    return BTreeNodeInsertMaxKey(desc, node, slot, pNewNode);
}
// 唯一索引，插入相同的key会core
void BTreeInsert(BTreeHandle handle, uint32_t key, uint8_t *value, uint32_t valueLen)
{
    BTreeDescT *desc = (BTreeDescT *)handle;
    BTreeSlotT slot = {
        .key = key,
        .value = value,
        .valueLen = valueLen
    };
    if (desc->root == NULL) {
        // 第一次插入数据
        desc->root = (BTreeNodeT *)BTreeNodeMalloc(desc->m);
        desc->first = desc->root;
        desc->last = desc->root;
        BTreeNodeInsertSlot(desc->root, 0, &slot, true);
    } else {
        BTreeNodeT *pNode = NULL;
        bool isSplit = BTreeNodeInsert(desc, desc->root, &slot, &pNode);
        if (isSplit) {
            BTreeNodeT *root = (BTreeNodeT *)BTreeNodeMalloc(desc->m);
            BTreeSlotT slot = {
                .key = desc->root->slot[desc->root->keyNum - 1].key,
                .child = desc->root
            };
            BTreeNodeInsertSlot(root, 0, &slot, false);
            root->level = desc->root->level + 1;
            slot.key = pNode->slot[pNode->keyNum - 1].key;
            slot.child = pNode;
            BTreeNodeInsertSlot(root, 1, &slot, false);
            desc->root = root;
        }
    }
}

void BTreeNodeDeleteSlot(BTreeNodeT *node, uint32_t index, bool isFree)
{
    if (isFree) {
        free(node->slot[index].value);
    }

    for (uint32_t i = index; i < node->keyNum - 1; i++) {
        node->slot[i] = node->slot[i + 1];
    }
    node->keyNum--;
}

// 借兄弟节点的第index个关键字
void BorrowFromSibling(BTreeNodeT *node, BTreeNodeT *sibling, BTreeSlotT *parentSlot, bool isLeftSibling)
{
    uint32_t targetIndex = 0; // 从左兄弟节点借的关键字，必然比当前节点的所有关键字小，右兄弟节点必然更大
    uint32_t slotIndex = sibling->keyNum - 1; // 左兄弟借最大的关键字，右兄弟借最小的关键字
    if (!isLeftSibling) {
        targetIndex = node->keyNum;
        slotIndex = 0;
        parentSlot->key = sibling->slot[slotIndex].key; // 借的关键字上升到父节点
    } else {
        parentSlot->key = sibling->slot[slotIndex - 1].key; // 剩下的关键字最大上升到父节点
    }
    
    BTreeNodeInsertSlot(node, targetIndex, &sibling->slot[slotIndex], false);
    BTreeNodeDeleteSlot(sibling, slotIndex, false);
}
// 把当前节点合并到兄弟节点
void MergeSibling(BTreeDescT *desc, BTreeNodeT *node, BTreeNodeT *sibling, BTreeSlotT *parentSlot, bool isLeftSibling)
{
    uint32_t slotIndex = sibling->keyNum; // 合并到左兄弟节点，关键字拷贝到sibling->keyNum的slot
    if (!isLeftSibling) {
        slotIndex = 0;
        memmove(&sibling->slot[node->keyNum], &sibling->slot[0], sibling->keyNum * sizeof(BTreeSlotT));
        if (node->level == 0) {
            sibling->prev = node->prev;
            node->prev->next = sibling;
            if (desc->first == node) {
                desc->first = sibling;
            }
        }
    } else {
        if (node->level == 0) {
            sibling->next = node->next;
            node->next->prev = sibling;
            if (desc->last == node) {
                desc->last = sibling;
            }
        }
    }
    memcpy(&sibling->slot[slotIndex], &node->slot[0], node->keyNum * sizeof(BTreeSlotT));
    sibling->keyNum += node->keyNum;
    parentSlot->key = sibling->slot[sibling->keyNum - 1].key;
    free(node);
}

// 发生合并
// 当parent = NULL时， index没有意义
bool BTreeNodeDelete(BTreeDescT *desc, BTreeNodeT *node, BTreeNodeT *parent, uint32_t index, uint32_t key)
{
    for (int i = 0; i < node->keyNum; i++) {
        if (key <= node->slot[i].key) {
            if (node->level != 0) {
                uint32_t oldMaxKey = node->slot[node->keyNum - 1].key;
                bool isMerge = BTreeNodeDelete(desc, node->slot[i].child, node, i, key);
                if (!isMerge) {
                    if (oldMaxKey == key && parent != NULL) { 
                        // 子节点没合并，最大关键值发生了变化，如果这个最大关键值也是当前节点的最大关键值，需要递归修改父节点的对应slot的关键字
                        parent->slot[index].key = node->slot[node->keyNum - 1].key;
                    }
                    return false;
                }
            } else {
                assert(key == node->slot[i].key);
            }
            BTreeNodeDeleteSlot(node, i, node->level == 0);
            if (parent == NULL) { // 当前节点是根节点
                return false;
            }
            if (node->keyNum >= desc->min) { // 当前节点满足B+树性质，返回false
                if (i == node->keyNum) { /*上面已经把关键字删了，所以要 + 1, 但是因为i是下标，所以判断相等就行*/
                    // 当前节点删除了最大关键值，需要修改父节点的关键字
                    parent->slot[index].key = node->slot[node->keyNum - 1].key;
                }
                return false;
            }
            // 当前节点不是父节点的第一个子节点，且左兄弟节点有关键字富余
            if (index != 0 && parent->slot[index - 1].child->keyNum > desc->min) {
                if (i == node->keyNum) { /*上面已经把关键字删了，所以要 + 1, 但是因为i是下标，所以判断相等就行*/
                    // 当前节点删除了最大关键值，需要修改父节点的关键字
                    parent->slot[index].key = node->slot[node->keyNum - 1].key;
                }
                BorrowFromSibling(node, parent->slot[index - 1].child, &parent->slot[index - 1], true);
                return false;   
            }
            // 当前节点不是父节点的最后一个孩子节点，且右兄弟节点有关键字富余
            if (index != parent->keyNum - 1 && parent->slot[index + 1].child->keyNum > desc->min) {
                BorrowFromSibling(node, parent->slot[index + 1].child, &parent->slot[index], false);
                return false;   
            }
            // 兄弟节点借不了，就只能合并了
            if (index != 0) { // 合并到左兄弟节点
                MergeSibling(desc, node, parent->slot[index - 1].child, &parent->slot[index - 1], true);
                return true;
            }
            // 合并到右兄弟节点
            MergeSibling(desc, node, parent->slot[index + 1].child, &parent->slot[index + 1], false);
            return true;
        }
    }
}

void BTreeDelete(BTreeHandle handle, uint32_t key)
{
    BTreeDescT *desc = (BTreeDescT *)handle;
    if (desc->root == NULL) {
        return;
    }
    BTreeNodeDelete(desc, desc->root, NULL, 0, key);
    if (desc->root->keyNum == 1 && desc->root->level != 0) {
        BTreeNodeT *root = desc->root->slot[0].child;
        free(desc->root);
        desc->root = root;
    }
}

int BTreeNodeLookUp(BTreeNodeT *node, uint32_t key, uint8_t **value)
{
    for (int i = 0; i < node->keyNum; i++) {
        if (key <= node->slot[i].key) {
            if (node->level == 0) {
                if (key != node->slot[i].key) {
                    return 0;
                }
                *value = node->slot[i].value;
                return 1;
            } else {
                int count = BTreeNodeLookUp(node->slot[i].child, key, value);
                return count == 0 ? 0 : count + 1;
            }
        }
    }
    return 0;
}
/*
    假设有n个关键字，一颗满B+树的层高（根节点为1）h = ln(n)/ln(m)
                    最高层高为h = ln(n)/ln((m+1)/2)
*/
// 返回访问过的节点数
int BTreeLookUp(BTreeHandle handle, uint32_t key, uint8_t **value)
{
    *value = NULL;
    BTreeDescT *desc = (BTreeDescT *)handle;
    if (desc->root == NULL) {
        return 0;
    }
    return BTreeNodeLookUp(desc->root, key, value);
}

void BTreeNodeDrop(BTreeNodeT *node)
{
    for (uint32_t i = 0; i < node->keyNum; i++) {
        if (node->level == 0) {
            free(node->slot[i].value);
        } else {
            BTreeNodeDrop(node->slot[i].child);
        }
    }
    free(node);
}

void BTreeDrop(BTreeHandle handle)
{
    BTreeDescT *desc = (BTreeDescT *)handle;
    if (desc->root == NULL) {
        return;
    }
    free(desc);
    LogClose();
}

// 可以重构下lookup和scan
void BTreeNodeFindKey(BTreeNodeT *node, int key, BTreeNodeT **locateNode)
{
    for (int i = 0; i < node->keyNum; i++) {
        if (key <= node->slot[i].key) {
            if (node->level == 0) {
                *locateNode = node;
            } else {
                BTreeNodeFindKey(node->slot[i].child, key, locateNode);
            }
            return;
        }
    }
}

void BTreeFindKey(BTreeHandle handle, int key, BTreeNodeT **node)
{
    BTreeDescT *desc = (BTreeDescT *)handle;
    if (desc->root == NULL) {
        *node = NULL;
        return;
    }
    BTreeNodeFindKey(desc->root, key, node);
}

void BTreeScanBegin(BTreeHandle handle, int keyBegin, int keyEnd, BTreeScanCursorT **cursor)
{
    BTreeScanCursorT *newCursor = AlgoMalloc(sizeof(BTreeScanCursorT));
    assert(newCursor != NULL);
    newCursor->desc = handle;
    newCursor->isEnd = false;
    newCursor->keyBegin = keyBegin;
    newCursor->keyEnd = keyEnd;
    newCursor->curNode = NULL;
    *cursor = newCursor;
    
}

bool BTreeScanFetch(BTreeScanCursorT *cursor, uint32_t *key, uint8_t **value, uint32_t *valueLen)
{
    if (cursor->isEnd) {
        return false;
    }
    BTreeNodeT *node = cursor->curNode;
    *key = node->slot[cursor->curSlotIndex].key;
    *value = node->slot[cursor->curSlotIndex].value;
    *valueLen = node->slot[cursor->curSlotIndex].valueLen;
}
// 返回true的时候，说明已经扫描完毕
bool BTreeScanMove(BTreeScanCursorT *cursor)
{
    if(cursor->isEnd) {
        return true;
    }
    BTreeNodeT *node = cursor->curNode;
    if (cursor->curNode == NULL) {
        BTreeNodeFindKey(cursor->desc->root, cursor->keyBegin, &cursor->curNode);
        if (cursor->curNode == NULL) {
            cursor->isEnd = true;
            return true;
        }
        int i = 0;
        for (; i < cursor->curNode->keyNum; i++) {
            if (cursor->keyBegin <= cursor->curNode->slot[i].key) {
                cursor->curSlotIndex = i;
                break;
            }
        }
        assert(i != cursor->curNode->keyNum);
        goto EXIT;
    }
    cursor->curSlotIndex++;
    if (cursor->curSlotIndex == node->keyNum) {
        if (cursor->desc->last == node) {
            cursor->isEnd = true;
            return true;
        } else {
            cursor->curSlotIndex = 0;
            cursor->curNode = node->next;
        }
    }
EXIT:
    if (cursor->curNode->slot[cursor->curSlotIndex].key > cursor->keyEnd) {
        cursor->isEnd = true;
        return true;
    }
    return false;
}

void BTreeScanEnd(BTreeScanCursorT *cursor)
{
    free(cursor);
}
