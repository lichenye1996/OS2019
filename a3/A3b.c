#include "rthread.h"

#define MIDDLE 0
#define HIGH 1
#define NLANES 7

struct swimmer{
    int middle_high;
    int last_middle_high;
    int is_head;
    struct swimmer* next;
};

struct pool
{
    rthread_lock_t lock;
    // you can add more monitor variables here
    rthread_cv_t lane_wait;
    rthread_cv_t swimmer_wait;
    struct swimmer* swimming_queue;
    int in;
    int in_middle_high;
};

void enqueue(struct swimmer** queue, struct swimmer* s)
{
    struct swimmer* temp = *queue; 
    s->next = NULL;
    // queue is empty
    if(*queue == NULL) {        
        *queue = s;
        (*queue)->is_head = 1;
        return;
    }

    // get the last element of the queue
    while(temp->next != NULL) {
        temp->next->last_middle_high = temp->middle_high;
        temp = temp->next;
    }

    // add s to the end of the queue
    temp->next = s;

    return;
}

struct swimmer* dequeue(struct swimmer** queue)
{
    struct swimmer* temp = *queue;
    // empty queue
    if(!temp) return NULL;
    // remove and return
    if (temp->next != NULL) temp->next->is_head = 1;
    *queue = temp->next;
    // set all the swimmers from same level as queue_heads (is_head = 1)
    while((*queue) != NULL && (*queue)->next != NULL && (*queue)->next->middle_high == (*queue)->middle_high){
        (*queue)->next->is_head = 1;
        (*queue) = (*queue)->next;
    }
    *queue = temp->next;
    return temp;

}

void pool_init(struct pool *pool)
{
    memset(pool, 0, sizeof(*pool));
    rthread_lock_init(&pool->lock);
    // initialize your monitor variables here
    rthread_cv_init(&pool->swimmer_wait, &pool->lock);
    rthread_cv_init(&pool->lane_wait, &pool->lock);
    pool->swimming_queue = NULL;
    pool->in = 0;
    pool->in_middle_high = -1;
}

void pool_enter(struct pool *pool, int level)
{
    rthread_with(&pool->lock)
    {
        // write the code here to enter the pool
        // initialize a swimmer structure
        struct swimmer* new_swimmer = malloc(5*sizeof(int));
        struct swimmer* swimmer_in = malloc(5*sizeof(int));
        new_swimmer->middle_high = level;
        new_swimmer->last_middle_high = -1;
        new_swimmer->is_head = 0;
        new_swimmer->next = NULL;
        // enqueue the swimmer
        enqueue(&pool->swimming_queue, new_swimmer);
        // wait conditions and dequeue
        while(!((pool->in == 0 || pool->swimming_queue->middle_high == pool->in_middle_high) && new_swimmer->is_head == 1)) {
            rthread_cv_wait(&pool->swimmer_wait);
        }
        swimmer_in = dequeue(&pool->swimming_queue);
        // check pool capacity (not necessary for the given conditions, but no harm)
        while(pool->in == NLANES) {
            rthread_cv_wait(&pool->lane_wait);
        }
        pool->in++;
        pool->in_middle_high = swimmer_in->middle_high;
    }
}

void pool_exit(struct pool *pool, int level)
{
    rthread_with(&pool->lock)
    {
        // write the code here to exit the pool
        pool->in--;
        rthread_cv_notifyAll(&pool->lane_wait);
        if(pool->in == 0){
            rthread_cv_notifyAll(&pool->swimmer_wait);
        }
    }
}