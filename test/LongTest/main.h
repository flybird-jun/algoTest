#ifndef MAIN_H
#define MAIN_H
#include "algo_typde_def.h"

class ModuleTest {
    void *TreadMain(void *arg) = 0;
    void *ConstructArg() = 0;
    void DestoryArg(void *arg) = 0;
};

#endif