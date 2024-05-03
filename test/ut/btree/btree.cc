#include "btree_pub.h"
#include <map>
#include <random>
#include "gtest/gtest.h"
using namespace testing;
using namespace std;

class BTreeTest : public testing::Test {
public:
    BTreeHandle handle;
    int m = 5;
    void SetUp() {
        BTreeCreate(m, &handle);
    }
    void TearDown() {
        BTreeDrop(handle);
    }
    static void SetUpTestCase() {

    }
    static void TearDownTestCase() {

    }
};

TEST_F(BTreeTest, Insert_001)
 {
    default_random_engine engine;
    engine.seed(time(0));
    map<uint32_t, void *> kvMap;
    // 随机生成100个随机数的key 和 100个随机value
    uint32_t count = 100;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t key = engine() % 1000;
        if (kvMap.find(key) == kvMap.end()) {
            uint32_t valueLen = engine() % 100 + 2; // value 长度1-100,包括"\0"
            uint8_t *value = new uint8_t[valueLen] {0};
            
            for (uint32_t j = 0; j < valueLen - 1; j++) {
                value[j] = engine() % 26 + 'a';
            }
            BTreeInsert(handle, key, value, valueLen);
            kvMap[key] = value;
        }
    }
    int minH = log(count)/log(m);
    int maxH = log(count)/log((m + 1) / 2);
    int level = 0;
    for (auto it : kvMap) {
        uint8_t *value = NULL;
        int level1 = BTreeLookUp(handle, it.first, &value);
        EXPECT_EQ(level == 0 || level == level1, true);
        level = level1;
        EXPECT_LE(level, maxH);
        EXPECT_GE(level, minH);
        int err = strcmp((char *)it.second, (char *)value);
        EXPECT_EQ(err, 0);
    }
}

/*
    ()内为nodeId
                                8|25(8)
             2|5|8(2)                                  11|14|17|20|25
    0|1|2   3|4|5(1)   6|7|8              9|10|11   12|13|14  15|16|17  18|19|20  21|22|23|24|25
*/
void Ut_BTreeDeleteKeys(BTreeHandle handle, uint32_t deleteKey[], uint32_t keyLen)
{
    for (uint32_t key = 0; key < 26; key++) {
        char value = 'a' + key;
        BTreeInsert(handle, key, (uint8_t *)&value, sizeof(char));
    }
    for (uint32_t i = 0; i < keyLen; i++) {
        BTreeDelete(handle, deleteKey[i]);
    }
    uint8_t *value;
    for (uint32_t key = 0; key < 26; key++) {
        int count = BTreeLookUp(handle, key, &value);
        if (count  == 0) {
            uint32_t index;
            for (index = 0; index < keyLen; index++) {
                if (key == deleteKey[index]) {
                    break;
                }
            }
            EXPECT_NE(index, keyLen);
        } else {
            EXPECT_EQ(*value, 'a' + key);
        }
    }
}

TEST_F(BTreeTest, Delete_001)
{
    uint32_t deleteKey[4] = {5, 11, 20, 25};
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}

// 删除key后，叶子节点仍然满足B+树性质
TEST_F(BTreeTest, Delete_002)
{
    uint32_t deleteKey[1] = {24};
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}

// 删除最大key后，叶子节点仍然满足B+树性质
TEST_F(BTreeTest, Delete_003)
{
    uint32_t deleteKey[1] = {25};
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}

// 删除叶子节点的非最大key后，叶子节点不满足B+树性质，且右兄弟节点富余，向右节点借关键字
TEST_F(BTreeTest, Delete_004)
{
    uint32_t deleteKey[1] = {16};
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}

// 删除叶子节点的最大key后，叶子节点不满足B+树性质，且右兄弟节点富余，向右节点借关键字
TEST_F(BTreeTest, Delete_005)
{
    uint32_t deleteKey[1] = {16};
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}

// 删除叶子节点的非最大key后，叶子节点不满足B+树性质，且左右兄弟节点都不富余， 向左节点合并
TEST_F(BTreeTest, Delete_006)
{
    uint32_t deleteKey[1] = {13};
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}

// 删除叶子节点的最大key后，叶子节点不满足B+树性质，且左右兄弟节点都不富余，向左节点合并
TEST_F(BTreeTest, Delete_007)
{
    uint32_t deleteKey[1] = {14};
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}

// 删除叶子节点的非最大key后，叶子节点不满足B+树性质，且左兄弟节点富余，向左节点借关键字
TEST_F(BTreeTest, Delete_008)
{
    uint32_t deleteKey[2] = {4, 6}; // 删除4后会向左节点合并，再删除6达到测试目的
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}
// 删除叶子节点的最大key后，叶子节点不满足B+树性质，且左兄弟节点富余，向左节点借关键字
TEST_F(BTreeTest, Delete_009)
{
    uint32_t deleteKey[2] = {4, 8}; // 删除4后会向左节点合并，再删除6达到测试目的
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}

// 删除叶子节点的非最大key后，叶子节点不满足B+树性质，且是父节点的第一个节点，右节点不富裕，向右节点合并
TEST_F(BTreeTest, Delete_010)
{
    uint32_t deleteKey[1] = {1};
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}
// 所有节点都删完
TEST_F(BTreeTest, Delete_011)
{
    uint32_t deleteKey[26];
    for (uint32_t i = 0; i < 26; i++) {
        deleteKey[i] = i;
    }
    Ut_BTreeDeleteKeys(handle, deleteKey, sizeof(deleteKey)/sizeof(uint32_t));
}

TEST_F(BTreeTest, SCAN_001)
{
    for (uint32_t key = 0; key < 26; key++) {
        char value = 'a' + key;
        BTreeInsert(handle, key, (uint8_t *)&value, sizeof(char));
    }
    BTreeScanCursorT *cursor;
    int begin = 5;
    int end = 20;
    BTreeScanBegin(handle, begin, end, &cursor);
    uint32_t i = 0;
    while(!BTreeScanMove(cursor)) {
        uint32_t key;
        char *value;
        uint32_t valueLen;
        bool eof = BTreeScanFetch(cursor, &key, (uint8_t **)&value, &valueLen);
        EXPECT_NE(eof, false);
        EXPECT_EQ(key, i + begin);
        EXPECT_EQ(*value, 'a' + key);
        i++;
    }
    EXPECT_EQ(i, 16);
}
