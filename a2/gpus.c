#include <stdio.h>
#include <stdlib.h>
#include "rthread.h"
rthread_lock_t print_lock;
// YOUR CODE GOES HERE
#define NGPUS 10
#include <string.h>
#include <assert.h>
struct gpu_info
{
    int allocated[NGPUS];
    unsigned int nfree;
    rthread_sema_t mutex;
    rthread_sema_t allocateSema;

    int waitRequests;
    int nAllocate;
};

void gi_init(struct gpu_info *gi);
void gi_alloc(struct gpu_info *gi,
              unsigned int ngpus,
              /* OUT */ unsigned int gpus[]);
void gi_release(struct gpu_info *gi,
                unsigned int ngpus,
                /* IN */ unsigned int gpus[]);
void gi_free(struct gpu_info *gi);
void gi_vacate(struct gpu_info *gi);



void gi_init(struct gpu_info *gi) {
    memset(gi->allocated, 0, sizeof(gi->allocated));
    gi->nfree = NGPUS;
    rthread_sema_init(&gi->mutex, 1);
    rthread_sema_init(&gi->allocateSema, 0);
    gi->waitRequests = 0;
    gi->nAllocate = 0;
}

void gi_vacate(struct gpu_info *gi){
    // Vacate Sema
    if (gi->nAllocate == 0 && gi->waitRequests > 0) {
        gi->waitRequests--;
        rthread_sema_vacate(&gi->allocateSema);
    }
    else {
        rthread_sema_vacate(&gi->mutex);
    }
}

void gi_alloc(struct gpu_info *gi, unsigned int ngpus, unsigned int gpus[]) {
    rthread_sema_procure(&gi->mutex);
    if (ngpus <= NGPUS) {
	// Accutal runnable threads
        if (ngpus > gi->nfree) {
            assert(ngpus <= NGPUS);
            assert(ngpus > gi->nfree);
            gi->waitRequests++;
            gi_vacate(gi);
            rthread_sema_procure(&gi->allocateSema);
        }
        
        gi->nAllocate++;
        gi->nfree -= ngpus;
        unsigned int g = 0;
        for (unsigned int i = 0; g < ngpus; i++) {
            assert(i < NGPUS);
            if (!gi->allocated[i]) {
                gi->allocated[i] = 1;
                gpus[g++] = i;
            }
        }
        gi_vacate(gi);
    }
    else {
	// Wait for infinite time if ngpus > NGPUS
        gi_vacate(gi);
	while(1);
    }
    
}

void gi_release(struct gpu_info *gi, unsigned int ngpus, /* IN */ unsigned int gpus[]) {
    rthread_sema_procure(&gi->mutex);
    for (unsigned int g = 0; g < ngpus; g++) {
        assert(gpus[g] < NGPUS);
        assert(gi->allocated[gpus[g]]);
        gi->allocated[gpus[g]] = 0;
    }
    gi->nfree += ngpus;
    assert(gi->nfree <= NGPUS);
    gi->nAllocate--;
    gi_vacate(gi);
}

void gi_free(struct gpu_info *gi) {
}



// =============================================================
//                  Test Code for GPU Allocator
// =============================================================
void gpu_user(void *shared, void *arg)
{
    struct gpu_info *gi = shared;
    unsigned int gpus[NGPUS];
    for (int i = 0; i < 5; i++)
    {
        rthread_delay(random() % 3000);
        unsigned int n = 1 + (random() % NGPUS);
        rthread_with(&print_lock)
        printf("%s %d wants %u gpus\n", (char *) arg, i, n);
        gi_alloc(gi, n, gpus);
        rthread_with(&print_lock)
        {
            printf("%s %d allocated:", (char *) arg, i);
            for (int i = 0; i < n; i++)
                printf(" %d", gpus[i]);
            printf("\n");
        }
        rthread_delay(random() % 3000);
        rthread_with(&print_lock)
            printf("%s %d releases gpus\n", (char *) arg, i);
        gi_release(gi, n, gpus);
    }
}
int main()
{
    rthread_lock_init(&print_lock);
    struct gpu_info gi;
    gi_init(&gi);
    rthread_create(gpu_user, &gi, "Jane");
    rthread_create(gpu_user, &gi, "Joe");
    rthread_run();
    gi_free(&gi);
    return 0;
}
