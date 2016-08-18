/*
 * key for qt
 *author:zz
 *  
*/


#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <linux/kdev_t.h>
#include<linux/err.h>
#include <linux/mutex.h>

#define TS_ENABLE 1
#define TS_UNABLE 0

/*zz 7-19+beep*/
/*buzzer beep when touch*/
#define GPIO_0 0x48032000
#define GPIO_1 0x4804C000
#define GPIO_IRQENABLE_SET_0 0X34
#define GPIO_IRQENABLE_SET_1 0X38
#define GPIO_IRQENABLE_CLR_0 0X3c
#define GPIO_IRQENABLE_CLR_1 0X40


#define TS_IRQ (59)

static int major;
static struct class *ts_class;
static int ts_irq;
static bool ts_status;
unsigned int *ts_2_ts;
static u32 ts_reg_read(u32 *ts_base,u32 offset)
{
	u32 address = (u32)ts_base+offset;
	return readl(address);
}

static void ts_reg_write(u32 *ts_base,u32 offset,u32 value)
{
	u32 address = (u32)ts_base+offset;
 	writel(value,address);
}

static irqreturn_t ts_irq_status(int irq, void *dev_id)
{
	
	BUG_ON(irq != gpio_to_irq(TS_IRQ));
		
	ts_status = true;
	return IRQ_HANDLED;
}

static ssize_t ts_open(struct inode  * inode, struct file * file)
{
	int error,irq_flags;
	//ts_irq =gpio_to_irq(TS_IRQ);
	ts_2_ts = (u32*)ioremap(GPIO_1,SZ_8K);
//	if (ts_irq < 0) {
	//		error = ts_irq;
	//		pr_info("Unable to get ts_irq number for GPIO %d, error %d\n",
	//			TS_IRQ, error);
	//		goto fail1;
//		}
	
		//irq_flags = IRQF_TRIGGER_FALLING  | IRQF_SHARED;//|IRQF_ONESHOT
		irq_flags = IRQF_SHARED;//|IRQF_ONESHOT
		error = request_irq(219, ts_irq_status, irq_flags, "ads7846",  &ts_2_ts);
	
		if (error) {
			pr_info("Unable to claim ts_irq %d; error %d\n",
				ts_irq, error);
			goto fail1;
		}

		ts_2_ts = (u32*)ioremap(GPIO_1,SZ_8K);

fail1:
	gpio_free(TS_IRQ);
	
	return 0;
}


static ssize_t ts_read(struct file * file, char __user *buf, size_t count, loff_t *ppos)
{
	
	int ret;
	int ts_value = 0;
#if 0
	if(!(file->f_flags & O_NONBLOCK))
		{
			wait_event_interruptible(key_wait, state );
		}
#endif

	if(ts_status)
		{
			ts_value = 1;
			ts_status = false;
		}
	else
			ts_value = 0;

		if (copy_to_user(buf, &ts_value, count)) {
			ret = -EFAULT;
			return ret;
				}


	return count;

	
}

static ssize_t ts_write(struct file *file, const char __user * user_buffer, size_t count, loff_t *ppos)

{
        int retval = 0;
	    char *buf = NULL;
	
	/* verify that we actually have some data to write */
	if (count != 0)
		{
		if (copy_from_user(buf, user_buffer, count))
			 {
			retval = -EFAULT;
			return retval;
			}
		}
	return count;
}


static long ts_ioctl(struct file *file,unsigned int cmd, unsigned long arg)
{
	int err;	
	unsigned int ret;
	
	switch (cmd) {
	case 	TS_UNABLE:
		ret = ts_reg_read(ts_2_ts,GPIO_IRQENABLE_CLR_0);
		ret |=1<<27;
		ts_reg_write(ts_2_ts,GPIO_IRQENABLE_CLR_0,ret);
		break;
	case TS_ENABLE:
		ret = ts_reg_read(ts_2_ts,GPIO_IRQENABLE_SET_0);
		ret |=1<<27;
		ts_reg_write(ts_2_ts,GPIO_IRQENABLE_SET_0,ret);
		break;
	default:
		pr_info("CMD err! TS_UNABLE  or TS_ENABLE");
		err = -1;
		break;
		}	
	
	iounmap(ts_2_ts);

		
	return err;
}

int ts_release(struct inode *inode, struct file *file)
{
	
	return 0;
}


static struct file_operations ts_openration ={
	.owner = THIS_MODULE,
	.open = ts_open,
	.read = ts_read,
	.write = ts_write,
	.unlocked_ioctl	= ts_ioctl,
	.release = ts_release,
};


static int __init ts_ctrl_init(void)
{
	int error = 0;
	
	major=register_chrdev( 0, "ts_ctrl",&ts_openration);
	if(!major)
		{
			pr_info("chrdev register error\n");
			goto fail1;
		}
	
		ts_class = class_create(THIS_MODULE, "ts_class");
		if (IS_ERR(ts_class)) {
		error = PTR_ERR(ts_class);
		goto fail2;
	}				
	device_create(ts_class, NULL, MKDEV(major, 0), NULL, "ts_ctrl"); 
		return 0;
fail2:
	unregister_chrdev(major, "ts_ctrl");
fail1:
	return error;
}

static void __exit ts_ctrl_exit(void)
{	
		device_destroy(ts_class,  MKDEV(major, 0));
		class_destroy(ts_class);
		unregister_chrdev(major, "ts_ctrl");
		pr_info("\"dev/ts_ctrl\" rmoved ok!\n");

}

module_init(ts_ctrl_init);
module_exit(ts_ctrl_exit);
MODULE_LICENSE("GPL");

