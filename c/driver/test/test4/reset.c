#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/hardware.h>
#include <linux/device.h>
#include <linux/gpio.h>


static struct class *gprs_class;


static ssize_t gprs_open(struct inode  * inode, struct file * file)
{
	printk("gprs_open\n\n");
	gpio_set_value(8,1);
	return 0;
}




static ssize_t  gprs_write(struct file * file, const char __user *buf, size_t count, loff_t *ppos)
{
	

	return 0;
}



static ssize_t gprs_read(struct file * file, char __user *buf, size_t count, loff_t *ppos)
{
	
	return 0;
}

static struct file_operations gprs_openration ={
	.owner = THIS_MODULE,
	.open = gprs_open,
	.write = gprs_write,
	.read = gprs_read,
};





static int major;
static int gprs_init(void)
{

	

	major=register_chrdev( 0, "restart",&gprs_openration);
	
	gprs_class = class_create(THIS_MODULE, "restart");
				    
	 device_create(gprs_class, NULL, MKDEV(major, 0), NULL, "test"); /* /dev/xyz */

	 gpio_request(8, "reset");
	
	/*c5300 power on  */
	gpio_direction_output(8,0);
	
						
	return 0;
}

static void gprs_exit(void)
{

	unregister_chrdev( major, "gprs");
	device_destroy(gprs_class, MKDEV(major, 0));    
	class_destroy(gprs_class);
	gpio_free(8);

	
	
}

module_init(gprs_init);
module_exit(gprs_exit);
MODULE_AUTHOR("zz");
MODULE_LICENSE("GPL");
