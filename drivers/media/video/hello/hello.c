#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>

static int_init hellomodule_init(void)
{
  printk("Hello Worid module init ok!\n");
  return 0;
}

static void_exit hellomodule_exit(void)
{
  printk("Hello Worid module eixt ok!\n");
  return 0;
}

module_init(hellomodule_init);
module_exit(hellomodule_exit);
MODULE_LICENSE("GPL");

