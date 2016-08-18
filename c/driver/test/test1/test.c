#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define TI8148_GPIO_0   0x48032000
#define TI8148_GPIO_1   0x4804C000
#define TI8148_GPIO_2   0x481AC000
#define GPIO_EOI 		0x20
#define GPIOIRQSATRUS_RAW_0	0X24
#define GPIOIRQSATRUS_RAW_1	0X28
#define GPIOIRQSATRUS_0	0X2C
#define GPIOIRQSATRUS_1	0X30
#define GPIO_IRQENABLE_SET_0	0x34
#define GPIO_IRQENABLE_SET_1    0x38
#define GPIO_IRQENABLE_CLR_0    0x3c
#define GPIO_IRQENABLE_CLR_1	0x40
#define GPIO_CTRL	0x130
#define GPIO_OE 0x134		/*output enable register*/
#define GPIO_DATAIN 0x138
#define GPIO_DATAOUT 0x13c
#define GPIO_LEVELDETECT0 0x140
#define GPIO_LEVELDETECT1	0x144
#define GPIO_RISINGDETECT  0x148
#define GPIO_FALLINGDETECT 0x14c
#define GPIO_DEBOUNCENABLE 0x150
#define GPIO_DEBOUNCINGTIME
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

static struct class *key_class;
static int major;
unsigned int *gp0_base;

/*register read and write function*/
static void key_reg_write(unsigned int *gpio_base,unsigned int offset,unsigned int value)
{
	unsigned int *address = gpio_base + offset;
	writel(value,address);
}
static unsigned int key_reg_read(unsigned int *gpio_base,unsigned int offset)
{
	unsigned int *address = gpio_base+offset;
	return readl(address);
}

/*clear interrupt*/
static void clear_irq(int flag)
{
	
}

static ssize_t key_write(struct file *file, const char __user *data,
				 size_t len, loff_t *ppos)
{
	

	printk("<kernel> call %s\n",__FUNCTION__);
	
	return 0;
}


static ssize_t key_read(struct file *file, char __user *buf,
				size_t len, loff_t *ppos)
{
	int ret;
	while(1)
		{
		ret = key_reg_read(gp0_base,GPIO_DATAIN);
			if(ret & (1<<27 |1<<25 |1<<24 |1<<22))
				printk("\nthis is key 10\n");
			msleep(10000);
			printk(KERN_INFO"#");
		}
	return 0;
}


static int key_open(struct inode *inode, struct file *file)
{

	

	printk("<kernel> call %s\n",__FUNCTION__);
	return 0;
}



static const struct file_operations key_fops = {
	.owner	= THIS_MODULE,//从属于当前模块
	.write	= key_write,
	.read	= key_read,
	.open	= key_open
	
};


static irqreturn_t key_irq(int irq, void *dev_id)
{
	char *str=(char *)dev_id;
	printk("interrupt occur....\n");
	printk("irq=%d dev_id=%s\n",irq,str);
	
	return IRQ_HANDLED;
}



static int key_init(void)
{	 
	int ret;
	int irq;
		major=register_chrdev( 0, "key_10",&key_fops);
		key_class = class_create(THIS_MODULE, "key_10");
		device_create(key_class, NULL, MKDEV(major, 0), NULL, "key_10"); /* /dev/xyz */
		
#if 1
	/*set gpio key interrupt*/
	gp0_base = (unsigned int *)ioremap(TI8148_GPIO_0,SZ_8K);
	ret = key_reg_read(gp0_base,GPIO_IRQENABLE_SET_0);
	ret  |= (1<<27 |1<<25 |1<<24 |1<<22);
	key_reg_write(gp0_base,GPIO_IRQENABLE_SET_0,ret);

	ret = key_reg_read(gp0_base,GPIO_RISINGDETECT);
	ret  |= (1<<27 |1<<25 |1<<24 |1<<22);
	key_reg_write(gp0_base,GPIO_RISINGDETECT,ret);
	irq = gpio_to_irq(27);
	ret=request_irq(irq,key_irq,IRQF_TRIGGER_RISING,"key_10","GPIO0INT0");
	irq = gpio_to_irq(27);
	ret=request_irq(irq,key_irq,IRQF_TRIGGER_RISING,"key_10","GPIO0INT0");
	if(ret<0){
		printk("<kernel>request irq fail %d\n",ret);
		return -EBUSY;
	}
		#endif
	ret = key_reg_read(gp0_base,GPIO_OE);
	ret  |= (1<<27 |1<<25 |1<<24 |1<<22);
	key_reg_write(gp0_base,GPIO_OE,ret);

	
	printk("<kernel>major=%d\n",major);
	printk("cdev:call key_init\n");

	return 0;
}

static void key_exit(void)
{
	dev_t devno;
	unregister_chrdev( major, "key_10");
	device_destroy(key_class, MKDEV(major, 0));    
	class_destroy(key_class);
	free_irq(96,"GPIO0INT0");
	iounmap(gp0_base);
	printk("call key_exit\n");

}


module_init(key_init);//强制告诉内核用的入口函数
module_exit(key_exit);//强制告诉内核用的出口函数
MODULE_LICENSE("GPL");//许可声明
MODULE_AUTHOR("zjq"); //作者信息
MODULE_VERSION("v_1.0");//版本信息;









