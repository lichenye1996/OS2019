#define WHISTLER        0
#define LISTENER        1
struct device {
       // your code here
       rthread_sema_t mutex;
       rthread_sema_t whistlerSema;
       rthread_sema_t listenerSema;
};

// Initialize the device
void dev_init(struct device *dev){
   rthread_sema_init(&dev->mutex, 1);
   rthread_sema_init(&dev->whistlerSema, 0);
   rthread_sema_init(&dev->listenerSema, 0);
}
void dev_enter(struct device *dev, int which);
void dev_exit(struct device *dev, int which);


