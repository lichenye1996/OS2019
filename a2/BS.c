#define WHISTLER        0
#define LISTENER        1
struct device {
       // your code here
};
   void dev_init(struct device *dev);
   void dev_enter(struct device *dev, int which);
   void dev_exit(struct device *dev, int which);