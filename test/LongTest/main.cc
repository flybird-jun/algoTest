#include <iostream>
using namespace std;
/*
    默认执行所有测试
    -m 指定模块名
    -t 测试时间 // 单位分钟,默认3分钟
*/
#define TEST_EXEC_TIME (60 * 3 * 1000)
static void RegisterModuleTest()
{
    ModuleFactory::RegisterModule("btree", []() -> ModuleTest * { new BTreeModuleTest()});
}

int main(int argc, char *argv[])
{
    RegisterModuleTest();

    uint32_t execTime = TEST_EXEC_TIME;
    vectory <ModuleTestBase *> moduleObj;
    // 处理命令
    if (argc >= 2) {
        // 从第二个参数开始
        for (int argIndex = 1; argIndex < argc; argIndex++) {
            if (strcmp(argv[argIndex], "-t") == 0) {
                if (argIndex + 1 == argc) {
                    cout << "parameter -t error" << endl;
                }
                execTime = atoi(argv[++argIndex]);
            }
            if (strcmp(argv[argIndex], "-m") {
                if (argIndex + 1 == argc) {
                    cout << "parameter -t error" << endl;
                }
                ModuleTestBase *testObj = CreateObj(argv[argIndex]);
                moduleObj.push(testObj);
            }
        }
    }
    if (moduleObj.size() == 0) {
        for (auto it : ModuleFactory::moduleMap) {
            ModuleTestBase *testObj = it->second();
            moduleObj.push(testObj);
        }
    }
    
    sleep(execTime); // 长稳测试时间
    for (auto it : moduleObj) {
        delete *it;
    }
}