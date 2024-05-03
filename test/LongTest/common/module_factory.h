#ifndef MAIN_H
#define MAIN_H
#include <map>
#include "module_test.h"
using namespace std;

class  ModuleFactory {
public:
    static void RegisterModule(char *mouldeName, void (*moduleCreate)());
    static ModuleTestBase* CreateObj(char *mouldeName);
    static map<char *, void (*)(void)>moduleMap;
};

#endif