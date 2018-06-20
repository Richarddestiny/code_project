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


#define GPIO_0 0x48032000
#define GPIO_OE 0x134 	/*output enable register*/
#define GPIO_DATAOUT 0x13c
#define GPIO_CLEARDATAOUT 0x190	/*wirte 1 to clear dataout register*/
#define GPIO_SETDATAOUT 0x194		/*write 1 to set dataout register*/ 

static struct class *gprs_class;



static u32 *gpio_data =NULL;


static u32 gpio_reg_read(u32 *gpio_base,u32 offset)
{
	u32 address = (u32)gpio_base+offset;

	return readl(address);

}

static void gpio_reg_write(u32 *gpio_base,u32 offset,u32 value)
{
	u32 address = (u32)gpio_base+offset;

	writel(value,address);

}

static ssize_t gprs_open(struct inode  * inode, struct file * file)
{
	printk("gprs_open\n\n");
	

	return 0;
}




static ssize_t  gprs_write(struct file * file, const char __user *buf, size_t count, loff_t *ppos)
{
	u32 reg = 0;
	/*
	if(!copy_from_user(&reg,buf,count))
			return -EFAULT;
		*/
		reg = 1;
	if(reg!=0)
		{
			reg=gpio_reg_read(gpio_data,GPIO_DATAOUT);/*power  io_16   reset io_27*/
			reg &= ~(1<< 27);
			gpio_reg_write(gpio_data,GPIO_DATAOUT,reg);

			msleep(150);
			
			reg=gpio_reg_read(gpio_data,GPIO_DATAOUT);/*power  io_16   reset io_27*/
			reg |=  (1<<27);
			gpio_reg_write(gpio_data,GPIO_DATAOUT,reg);
		}
	else
		{
			
			printk("function write nothing to do\n");
			
		}

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

	
	u32 reg;

	major=register_chrdev( 0, "gprs",&gprs_openration);
	
	gprs_class = class_create(THIS_MODULE, "gprs");
				    
	 device_create(gprs_class, NULL, MKDEV(major, 0), NULL, "c5300"); /* /dev/xyz */

	gpio_data = (u32*)ioremap(GPIO_0,SZ_8K);
	
	/*c5300 power on  */
	
	reg=gpio_reg_read(gpio_data,GPIO_OE);/*power  io_16   reset io_27*/
	
	reg &= ~(1<< 15|1<<16);
	gpio_reg_write(gpio_data,GPIO_OE,reg);//output enable/
	reg=gpio_reg_read(gpio_data,GPIO_DATAOUT);//power  io_15   reset io_16
	reg &= ~(1<<16); //output high to hold the rest/
	reg |= (1<< 15); //output high to power on/
	gpio_reg_write(gpio_data,GPIO_DATAOUT,reg);
	
	/*c5300 power on*/
						
	return 0;
}

static void gprs_exit(void)
{
	u32 reg;
	unregister_chrdev( major, "gprs");
	device_destroy(gprs_class, MKDEV(major, 0));    
	class_destroy(gprs_class);


	/*power off*/	
	reg=gpio_reg_read(gpio_data,GPIO_DATAOUT);
	reg &= ~(1<< 15);
	gpio_reg_write(gpio_data,GPIO_DATAOUT,reg);
	
	iounmap(gpio_data);	
}

module_init(gprs_init);
module_exit(gprs_exit);
MODULE_AUTHOR("zz");
MODULE_LICENSE("GPL");
