#ifndef MODULE_TEST_H
#define MODULE_TEST_H
#include <pthread.h>
class ArgBase {
public:
    bool isRunning;
    virtual ~ArgBase() {}
};
class ModuleTestBase {
public:
    ModuleTestBase() {
        void *arg = ConstructArg();
        pthread_create(&tid, NULL, TreadMain, arg);
    }
    void *TreadMain(void *arg) = 0;
    ArgBase *ConstructArg() = 0;
    void DestoryArg(ArgBase *arg) = 0;
    virtual ~ModuleTestBase() {
        arg->isRunning = false;
        pthread_join(tid, NULL);
        DestoryArg(arg);
    }
private:
    pthread_t tid;
};

#endif