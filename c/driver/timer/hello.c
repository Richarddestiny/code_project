#include<linux/init.h>
#include<linux/module.h>
#include<linux/delay.h>
#include<linux/timer.h>
#include<linux/jiffies.h>

struct timer_list timer;

void timer_fun(unsigned long data)
{
	printk("boom\n");

	mod_timer(&timer,jiffies+2*HZ);	

}

static int hello_init(void)
{
	printk("call hello_init\n");
	printk("start delay jiffies:%ld\n",jiffies);
	mdelay(1000);
	printk("hello time jiffies:%ld\n",jiffies);
	init_timer(&timer);
	timer.expires=jiffies+5*HZ;
	timer.function=timer_fun;
	timer.data=100;
	add_timer(&timer);
	
	return 0;
}

static void hello_exit(void)
{
	del_timer(&timer);
	printk("call hello_exit\n");

}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("farsight");
MODULE_VERSION("1.0");







