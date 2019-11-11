#include "rthread.h"

#define MIDDLE 0
#define HIGH 1
#define NLANES 7

struct pool
{
    rthread_lock_t lock;
    // you can add more monitor variables here
    rthread_cv_t team_wait;
    rthread_cv_t lane_wait;
    int middle_high;
    int in;

};

void pool_init(struct pool *pool)
{
    memset(pool, 0, sizeof(*pool));
    rthread_lock_init(&pool->lock);
    // initialize your monitor variables here
    rthread_cv_init(&pool->team_wait, &pool->lock);
    rthread_cv_init(&pool->lane_wait, &pool->lock);
    pool->middle_high = -1;
    pool->in = 0;

}

void pool_enter(struct pool *pool, int level)
{
    rthread_with(&pool->lock)
    {
        // write the code here to enter the pool
        if (pool->middle_high >= 0) {
            while(pool->middle_high != level && pool->middle_high != -1){
                rthread_cv_wait(&pool->team_wait);
            }
            while(pool->in == NLANES) {
                rthread_cv_wait(&pool->lane_wait);
            }
            pool->in++;
        }
        else {
            pool->middle_high = level;
            pool->in++;
        }
    }
}

void pool_exit(struct pool *pool, int level)
{
    rthread_with(&pool->lock)
    {
        // write the code here to exit the pool
        pool->in--;
        rthread_cv_notifyAll(&pool->lane_wait);
        if (pool->in == 0) {
            pool->middle_high = -1;
            rthread_cv_notifyAll(&pool->team_wait);
        }

        
    }
}