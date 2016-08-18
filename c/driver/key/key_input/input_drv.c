/*
 * key for qt
 *author:zz
 *  
*/

#include<linux/module.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/input.h>
#include<linux/irqreturn.h>
#include<asm/arch/regs-gpio.h>
#include<asm/arch/gpio.h>



#define TI8148_GPIO_0   0x48032000
#define TI8148_GPIO_1   0x4804C000
#define TI8148_GPIO_2   0x481AC000
#define GPIO_OE 0x134		/*output enable register*/
#define GPIO_DATAIN 0x138
#define GPIO_DATAOUT 0x13c
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194



/*定义一个结构体用来描述行线*/
struct cow_pin {
	int io;
	int io_key;
	//int irq;
	char *name;
	int key_val;
	int pin_state;
};

static struct cow_pin key_desc[10]={
	{TI8148_GPIO_0,27,"k01",KEY_L},
	{TI8148_GPIO_0,08,"K02",KEY_L},
	{TI8148_GPIO_0,25,"K03",KEY_L},
	{TI8148_GPIO_0,24,"K04",KEY_L},
	{TI8148_GPIO_2,02,"K05",KEY_L},
	{TI8148_GPIO_0,22,"K06",KEY_L},
	{TI8148_GPIO_0,21,"K07",KEY_L},
	{TI8148_GPIO_0,15,"K08",KEY_S},
	{TI8148_GPIO_0,19,"K09",KEY_ENTER},
	{TI8148_GPIO_1,07,"K10",KEY_LEFTSHIFT},		
};

static struct cow_pin *cur_pin;
/*定义一个内核定时器*/
static struct timer_list ti8148_key_time;

struct input_dev *ti8148_input_key;
static wait_queue_head_t key_wait;

#if 0
struct input_dev {

	void *private;

	const char *name;
	const char *phys;
	const char *uniq;
	struct input_id id;

	unsigned long evbit[NBITS(EV_MAX)];
	unsigned long keybit[NBITS(KEY_MAX)];
	unsigned long relbit[NBITS(REL_MAX)];
	unsigned long absbit[NBITS(ABS_MAX)];
	unsigned long mscbit[NBITS(MSC_MAX)];
	unsigned long ledbit[NBITS(LED_MAX)];
	unsigned long sndbit[NBITS(SND_MAX)];
	unsigned long ffbit[NBITS(FF_MAX)];
	unsigned long swbit[NBITS(SW_MAX)];

	struct list_head	h_list;
	struct list_head	node;
};
K10->L
K7->S
K4->ENTER
K1->LEFTSHIFT
#endif
/*register read and write function*/
static void key_reg_write(unsigned int *gpio_base,unsigned int offset,unsigned int value)
{
	unsigned int address = gpio_base + offset;
	writel(value,address);
}
static unsigned int key_reg_read(unsigned int *gpio_base,unsigned int offset)
{
	unsigned int address = gpio_base+offset;
	return readl(address);
}

/*kernel timer funciotion*/
static void ti8148_key_timer_fun(unsigned long data)
{
	unsigned long val;
	if(!cur_pin)
		return;
	
	/*get pin satate  second time*/
	val =s3c2410_gpio_getpin(cur_pin->pin);

	/*judge*/
	if(val !=cur_pin->pin_state)	
		return;
	
	/*如果一样就说明真的是认为按下*/
	if(val)
	{
		/*松开*/
		/*上报事件*/
		/*value:0->松开  1->按下*/
		
		input_event(ti8148_input_key,EV_KEY,cur_pin->key_val,0);
		//通知上层,事件上报完成
		input_sync(ti8148_input_key);
	}
	else
	{
		/*按下*/
		input_event(ti8148_input_key,EV_KEY,cur_pin->key_val,1);
		input_sync(ti8148_input_key);
	}
	
}

/*实现中断服务程序组*/
static void  ti8148_key_wait(void)
{
while(1)
{
	DECLARE_WAITQUEUE(wait, current);
	cur_pin =(struct cow_pin *)dev_id;
	/*1.第一次获取管脚状态*/
	cur_pin->pin_state	=s3c2410_gpio_getpin(cur_pin->pin);

	/*2.为了消抖,需要重载定时器*/
	mod_timer(&ti8148_key_time,jiffies +5);
}	
}

static int __init ti8148_key_init(void)
{
	int i;
	int ret;
	unsigned int *gp0_base,*gp1_base,*gp2_base;

	ti8148_input_key = input_allocate_device();
	set_bit(EV_KEY, ti8148_input_key->evbit);
	set_bit(KEY_L,ti8148_input_key->keybit);
	input_register_device(ti8148_input_key);

	init_timer(&ti8148_key_time);
	ti8148_key_time.expires		=jiffies + 5;
	ti8148_key_time.function	=ti8148_key_timer_fun;
	add_timer(&ti8148_key_time);

	/*init queue head*/
	init_waitqueue_head(key_wait);
	/*set key pins to input*/
	gp0_base = (unsigned int)ioremap(TI8148_GPIO_0,SZ_8K);
	gp1_base = (unsigned int)ioremap(TI8148_GPIO_1,SZ_8K);
	gp2_base = (unsigned int)ioremap(TI8148_GPIO_2,SZ_8K);

	ret = key_reg_read(gp0_base,GPIO_OE);
	ret  |= (1<<8)|(1<<27)(1<<25)(1<<24)(1<<22)(1<<21)(1<<15)(1<<19);
	key_reg_write(gp0_base,GPIO_OE,ret);

	ret = key_reg_read(gp1_base,GPIO_OE);
	ret  |= (1<<7);
	key_reg_write(gp1_base,GPIO_OE,ret);

	ret = key_reg_read(gp2_base,GPIO_OE);
	ret  |= (1<<2);
	key_reg_write(gp2_base,GPIO_OE,ret);
	
	

	return 0;
}

/*实现出口函数*/
static void __exit ti8148_key_exit(void)
{
	int i;
	del_timer(&ti8148_key_time);
	for(i=0;i<4;i++)
		free_irq(key_desc[i].irq,&key_desc[i]);
	input_unregister_device(ti8148_input_key);
	input_free_device(ti8148_input_key);
}

module_init(ti8148_key_init);
module_exit(ti8148_key_exit);
MODULE_LICENSE("GPL");

