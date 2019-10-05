#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rthread.h"

#define WHISTLER 0
#define LISTENER 1
struct device
{
    // your code here
    rthread_sema_t mutex;
    rthread_sema_t whistlerSema;
    rthread_sema_t listenerSema;

    // keep track of the #
    int nWhistlerEntered, nWhistlerWaiting;
    int nListenerEntered, nListenerWaiting;
};

void dev_init(struct device *dev);
void dev_enter(struct device *dev, int which);
void dev_exit(struct device *dev, int which);
void dev_vacateOne(struct device *dev);

void dev_init(struct device *dev) {
   rthread_sema_init(&dev->mutex, 1);
   rthread_sema_init(&dev->whistlerSema, 0);
   rthread_sema_init(&dev->listenerSema, 0); 
   dev->nWhistlerEntered = 0;
   dev->nWhistlerWaiting = 0;
   dev->nListenerEntered = 0;
   dev->nListenerWaiting = 0;
}

void dev_vacateOne(struct device *dev){
    // No whistler entered, and waiting listener
    if (dev->nWhistlerEntered == 0 && dev->nListenerWaiting > 0) {
        dev->nListenerWaiting--;
        rthread_sema_vacate(&dev->listenerSema);
    }
    // No listener entered, and waiting whistler
    else if (dev->nListenerEntered == 0 && dev->nWhistlerWaiting > 0) {
        dev->nWhistlerWaiting--;
        rthread_sema_vacate(&dev->whistlerSema);
    }
    else {
        rthread_sema_vacate(&dev->mutex);
    }
}


void dev_enter(struct device *dev, int which) {
    rthread_sema_procure(&dev->mutex);
    // Check the Invariant
    assert((dev->nWhistlerEntered >=0 && dev->nListenerEntered == 0) || (dev->nWhistlerEntered == 0 && dev->nListenerEntered >= 0));
    // Whistler
    if (which == 0) {
        if (dev->nListenerEntered > 0){
            dev->nWhistlerWaiting++;
            dev_vacateOne(dev);
            rthread_sema_procure(&dev->whistlerSema);
        }
        assert(dev->nListenerEntered == 0);
        dev->nWhistlerEntered++;
        dev_vacateOne(dev);
    }
    // Listener
    else if (which == 1) {
        if (dev->nWhistlerEntered > 0){
            dev->nListenerWaiting++;
            dev_vacateOne(dev);
            rthread_sema_procure(&dev->listenerSema);
        }
        assert(dev->nWhistlerEntered == 0);
        dev->nListenerEntered++;
        dev_vacateOne(dev);
    }
    else {
        assert(which != 1 && which != 0);
    }
    


}

void dev_exit(struct device *dev, int which) {
    rthread_sema_procure(&dev->mutex);
    if (which == 0){
        assert(dev->nListenerEntered == 0);
        assert(dev->nWhistlerEntered > 0);
        dev->nWhistlerEntered--;
        dev_vacateOne(dev);
    }
    else if (which == 1){
        assert(dev->nWhistlerEntered == 0);
        assert(dev->nListenerEntered > 0);
        dev->nListenerEntered--;
        dev_vacateOne(dev);
    }
    else {
        assert(which != 1 && which != 0);
    }

}

// =============================================================
//          Test Code for Lab of Ornithology IoT Device
// =============================================================

#define NWHISTLERS 3
#define NLISTENERS 3
#define NEXPERIMENTS 2

char *whistlers[NWHISTLERS] = {"w1", "w2", "w3"};
char *listeners[NLISTENERS] = {"l1", "l2", "l3"};

void worker(void *shared, void *arg)
{
    struct device *dev = shared;
    char *name = arg;
    for (int i = 0; i < NEXPERIMENTS; i++)
    {
        printf("worker %s waiting for device\n", name);
        dev_enter(dev, name[0] == 'w');
        printf("worker %s has device\n", name);

        rthread_delay(random() % 3000);
        printf("worker %s releases device\n", name);
        dev_exit(dev, name[0] == 'w');
        rthread_delay(random() % 3000);
    }
    printf("worker %s is done\n", name);
}

int main()
{
    struct device dev;
    dev_init(&dev);
    for (int i = 0; i < NWHISTLERS; i++)
    {
        rthread_create(worker, &dev, whistlers[i]);
    }
    for (int i = 0; i < NLISTENERS; i++)
    {
        rthread_create(worker, &dev, listeners[i]);
    }
    rthread_run();
    return 0;
}
