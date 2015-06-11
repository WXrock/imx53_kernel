#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>		//for register_chrdev(), file_opreation[]
#include <linux/init.h>		//for __init, __exit
#include <linux/delay.h>	//mdelay(), ndelay(), ssleep(),
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <linux/pci.h>
#include <linux/gpio.h>
//#include <plat/gpio-core.h>        
//#include <plat/gpio-cfg.h>
//#include <plat/gpio-cfg-helpers.h>

#include <mach/iomux-mx53.h>	
#include <mach/gpio.h>		// arch/arm/plat-mxc/include/mach/*,gpio_set_value()
#include <asm/irq.h>

#define GPIO_MAGIC 'G'
#define GPIO_GET_DBGCTRL   0x40044700
#define GPIO_SET_DBGCTRL   0x40044701
#define GPIO_GET_CFGPIN   0x40044702
#define GPIO_SET_CFGPIN   0x40044703
#define GPIO_GET_PULL  0x40044704
#define GPIO_SET_PULL  0x40044705
#define GPIO_MAXNR 5


#define PLOG(fmt, args...) \
	do { \
	if(dbg_enable) \
	printk(fmt, ##args); \
	} while (0)

//
static int bjw_gpio_read(struct file *filp,char __user *buf_user,size_t size,loff_t *offset)
{
	char cmd[16];
	
	int ret,pin_port,pin_num,pin_io,val;
	ret=copy_from_user(cmd,buf_user,size);  
	if(ret!=0)
		return -EINVAL;
     //get pin_port and pin_num from cmd
	sscanf(cmd, "%d %d", &pin_port, &pin_num); 
	//PLOG("cmd:get port%d.%d value\n", pin_port, pin_num);
	pin_io=(pin_port-1)*32+pin_num;   
	if(pin_io<0)
		return pin_io;
	gpio_request(pin_io, cmd); 
	val = gpio_get_value(pin_io); 
	gpio_free(pin_io); 
	//PLOG("Get port%d.%d value = %d\r\n",pin_port,pin_num,val);
	return val;
}

static int bjw_gpio_write(struct file *filp,const char __user *buf_user,size_t size,loff_t *offset)
{
	char cmd[16];
	int ret,pin_port,pin_num,pin_io,val;
	ret=copy_from_user(cmd,buf_user,size);
	if(ret!=0)
		return -EINVAL;
	sscanf(cmd, "%d %d %d", &pin_port, &pin_num,&val);
	//PLOG("cmd: set %d.%d value %d\n", pin_port, pin_num,val);
	pin_io=(pin_port-1)*32+pin_num;   
	if(pin_io<0)
		return pin_io;
	gpio_request(pin_io, cmd);
	gpio_set_value(pin_io,val);
	gpio_free(pin_io); 
	//PLOG("Set %d.%d value = %d\n", pin_port, pin_num, val);
	return pin_io;
} 
 
struct gpio_pin_data {
char* pin_group;
int pin_num;
int pin_data;
};

struct gpio_info {
char* pin_group;
int pin_num;
};

static long bjw_gpio_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
//	int pin_io;
//	unsigned ret;
	//s3c_gpio_pull_t ret_pull;
//	PLOG("GPIO_GET_DBGCTRL  = %x,GPIO_SET_DBGCTRL=%x\n",GPIO_GET_DBGCTRL,GPIO_SET_DBGCTRL);
//	PLOG("GPIO_GET_CFGPIN  = %x,GPIO_SET_CFGPIN=%x\n",GPIO_GET_CFGPIN,GPIO_SET_CFGPIN);
//	PLOG("GPIO_GET_PULL  = %x,GPIO_SET_PULL=%x\n",GPIO_GET_PULL,GPIO_SET_PULL);
	/*if(_IOC_TYPE(cmd)!=GPIO_MAGIC) return -EINVAL;
	if(_IOC_NR(cmd)>GPIO_MAXNR || _IOC_NR(cmd)<0)
		return -EINVAL;
	switch (cmd) {

	case GPIO_SET_DIR_IN:
	{
		struct gpio_info pin_info;
		PLOG("GPIO_GET_CFGPIN \r\n");
		if(copy_from_user(&pin_info,(const char*)arg,sizeof(pin_info)) !=0)
			return -EINVAL;
		PLOG("cmd:Get %s(%d) cfgpin \r\n",pin_info.pin_group,pin_info.pin_num);
		pin_io=bjw_gpio_get_pin_io(pin_info.pin_group,pin_info.pin_num);
		if(pin_io<0)
			return pin_io;
		gpio_request(pin_io, pin_info.pin_group);
		ret=s3c_gpio_getcfg(pin_io);
		ret=ret & 0xf;
		PLOG("get pin_cfg:%d \r\n",ret);
		gpio_free(pin_io);
		return ret;
	}
	
	default:
		PLOG("Can't Find cmd:%x\n", cmd);
	break;
	}*/	
	return 0;
}


static struct file_operations dev_fops = {
    .owner   =   THIS_MODULE,
   .ioctl= bjw_gpio_ioctl,//if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
    //.unlocked_ioctl= bjw_gpio_ioctl,
    .read    =   bjw_gpio_read,
    .write   =   bjw_gpio_write,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bjw-gpio",
	.fops = &dev_fops,
};

static int bjw_gpio_probe(struct platform_device *dev)
{
	int ret;
	//PLOG("bjw platform dirver find bjw platfrom device.\n");
	ret = misc_register(&misc);
	if(ret)
		return ret;
	return 0;
}

static int bjw_gpio_remove(struct platform_device *pdev)
{
	//PLOG("bjw GPIO platfrom device has removed.\n");
	misc_deregister(&misc);
	return 0;
}
//
static struct platform_device  bjw_gpio_device= {
        .name = "bjw-gpio",
        .id   = -1,
};
//
static struct platform_driver bjw_gpio_driver = {
	.probe	= bjw_gpio_probe,
	.remove	= bjw_gpio_remove,
	.driver	= {
		.name	= "bjw-gpio",
		.owner = THIS_MODULE,
	},
};
//
int __init bjw_gpio_init(void)
{
        platform_device_register(&bjw_gpio_device);
        platform_driver_register(&bjw_gpio_driver);
        return 0;
}
void __exit bjw_gpio_exit(void)
{
	platform_driver_unregister(&bjw_gpio_driver);
	platform_device_unregister(&bjw_gpio_device); 
}

module_init(bjw_gpio_init);
module_exit(bjw_gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MXR&TT");
MODULE_DESCRIPTION("GPIO for FreeScale imx53");
MODULE_VERSION("0.1");
