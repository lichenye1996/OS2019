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
    //printf("**** queue is %d *****\n",*queue==NULL);
    if(*queue == NULL) {
        
        *queue = s;
        (*queue)->is_head = 1;

        printf("====================================\nthis is my queue: \n");
        struct swimmer* temp2 = *queue;
        printf("%d \n", temp2->middle_high);
        printf("%d \n", temp2->last_middle_high);
        printf("is head %d", temp2->is_head);
        printf("\n===================================\n");

        return;
    }

    // get the last element of the queue
    while(temp->next != NULL) {
        temp->next->last_middle_high = temp->middle_high;
        temp = temp->next;
    }
    
    // add s to the end of the queue
    temp->next = s;
    printf("====================================\nthis is my queue: \n");
    struct swimmer* temp2 = *queue;
    while(temp2->next != NULL) {
        printf("%d ", temp2->middle_high);
        temp2 = temp2->next;
    }
    printf("\n");
    struct swimmer* temp3 = *queue;
    while(temp3->next != NULL) {
        printf("%d ", temp3->last_middle_high);
        temp3 = temp3->next;
    }
    printf("\n");
    struct swimmer* temp4 = *queue;
    while(temp4->next != NULL) {
        printf("%d ", temp4->is_head);
        temp4 = temp4->next;
    }
    printf("\n===================================\n");

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
    
    struct swimmer*** temp2 = &queue;
    //printf("queue address is %p\n", temp2);
    while((*queue) != NULL && (*queue)->next != NULL && (*queue)->next->middle_high == (*queue)->middle_high){
        (*queue)->next->is_head = 1;
        (*queue) = (*queue)->next;
    }
    *queue = temp->next;
    // printf("heads assign done........\n");
    return temp;

}

void set_queue_heads(struct swimmer** queue)
{
    struct swimmer*** temp = &queue;
    while((**temp) != NULL && (**temp)->next->middle_high == (**temp)->middle_high) {
        (**temp)->next->is_head = 1;
        (**temp) = (**temp)->next;
    }
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
        struct swimmer* new_swimmer = malloc(5*sizeof(int));
        struct swimmer* swimmer_in = malloc(5*sizeof(int));
        new_swimmer->middle_high = level;
        new_swimmer->last_middle_high = -1;
        new_swimmer->is_head = 0;
        new_swimmer->next = NULL;
        enqueue(&pool->swimming_queue, new_swimmer);
        //printf("&&&&&&& Dequeue check result is %d &&&&&&&&&\n", !(pool->in == 0 || pool->swimming_queue->middle_high == pool->in_middle_high));
        while(!((pool->in == 0 || pool->swimming_queue->middle_high == pool->in_middle_high) && new_swimmer->is_head == 1)) {
            rthread_cv_wait(&pool->swimmer_wait);
        }
        // if(pool->swimming_queue->middle_high){
        //     printf("xxxxxxxxxxxxxxx h released! with pool_in = %d, in_middle_high = %d, is_head = %d\n", pool->in, pool->in_middle_high, new_swimmer->is_head);
        // }
        // else{
        //     printf("%%%%%%%%%%%%%% m released! with pool_in = %d, in_middle_high = %d, is_head = %d\n", pool->in, pool->in_middle_high, new_swimmer->is_head);
        // }
        swimmer_in = dequeue(&pool->swimming_queue);

        while(pool->in == NLANES) {
            rthread_cv_wait(&pool->lane_wait);
        }
        pool->in++;
        pool->in_middle_high = swimmer_in->middle_high;
        if(level){
            printf("............h has entered!\n");
        }
        else{
            printf("............m has entered!\n");
        }
        


    }
}

void pool_exit(struct pool *pool, int level)
{
    rthread_with(&pool->lock)
    {
        // write the code here to exit the pool
        printf("exit point......\n");
        pool->in--;
        rthread_cv_notifyAll(&pool->lane_wait);
        if(pool->in == 0){
            rthread_cv_notifyAll(&pool->swimmer_wait);
        }
        if(level){
            printf("............h has left!\n");
        }
        else{
            printf("............m has left!\n");
        }
        

    }
}