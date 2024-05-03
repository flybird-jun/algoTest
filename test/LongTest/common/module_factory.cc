#include "module_factory.h"

void ModuleFactory::RegisterModule(char *moduleName, void (*moduleCreate)())
{
    moduleMap[moduleName] = moduleCreate;
}

ModuleTestBase* ModuleFactory::CreateObj(char *mouldeName)
{
    assert(moduleMap[moduleName] != NULL);
    return moduleMap[moduleName]();
}