typedef rthread_lock_t, rthread_sema_t;
void rthread_create(
void (*f)(void *shared, void *arg),
void *shared,
void *arg);
void rthread_lock_init(rthread_lock_t *lock);
void rthread_sema_init(rthread_sema_t *sema, unsigned init);
void rthread_sema_procure(rthread_sema_t *sema);
void rthread_sema_vacate(rthread_sema_t *sema);
void rthread_delay(unsigned int milliseconds);
void rthread_run(void);