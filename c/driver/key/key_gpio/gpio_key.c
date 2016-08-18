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
#include <linux/interrupt.h>

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
#define GPIO_OE 0x134 	/*output enable register*/
#define GPIO_DATAOUT 0x13c
#define KEY_MUX 0x48140AB8



struct keys_platform_data {
	struct gpio_keys_button *buttons;
	int nbuttons;
	unsigned int poll_interval;	/* polling interval in msecs -
					   for polling driver only */
	unsigned int rep:1;		/* enable input subsystem auto repeat */
	int (*enable)(struct device *dev);
	void (*disable)(struct device *dev);
};

struct gpio_keys_button {
	/* Configuration parameters */
	int code;		/* input event code (KEY_*, SW_*) */
	int gpio;
	int active_low;
	char *desc;
	int type;		/* input event type (EV_KEY, EV_SW) */
	int wakeup;		/* configure the button as a wake-up source */
	int debounce_interval;	/* debounce ticks interval in msecs */
	bool can_disable;
};

struct gpio_key_data {
	struct gpio_keys_button *button;
	struct timer_list timer;
	int timer_debounce;	/* in msecs */
	bool disabled;
};

struct gpio_keys_datadrv {
	struct mutex disable_lock;
	unsigned int n_buttons;
	int (*enable)(struct device *dev);
	void (*disable)(struct device *dev);
	struct gpio_key_data data[0];
};

static u32 *gpio_data =NULL;
static unsigned int *key_mux = NULL;
static wait_queue_head_t key_wait;
static int major;
static struct class *keys_class;

static bool state = 0;
static bool able =0;

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

static void buzzer_beep(void)
{
	u32 reg;
	reg=gpio_reg_read(gpio_data,GPIO_DATAOUT);
	reg |= (1<< 10);
	gpio_reg_write(gpio_data,GPIO_DATAOUT,reg);
	msleep(50);
	reg &= ~(1<< 10);
	gpio_reg_write(gpio_data,GPIO_DATAOUT,reg);
}
static int buzzer_beep_init(void)
{
	u32 reg;
	gpio_data = (u32*)ioremap(GPIO_0,SZ_8K);
	if(!gpio_data)
			return  -1;
	reg=gpio_reg_read(gpio_data,GPIO_OE);
	reg &= ~(1<< 10);
	gpio_reg_write(gpio_data,GPIO_OE,reg);/*output enable*/
	return 0;
}
static void buzzer_beep_detory(void)
{
	iounmap(gpio_data);
}

struct get_key_struct {
	int gpio;
	int code;
	int value;
};

struct get_key_struct get_my_key ;

static void key_respone(struct gpio_key_data *key)
{
	get_my_key.code = key->button->code;
	get_my_key.gpio = key->button->gpio;
	get_my_key.value = gpio_get_value(key->button->gpio);
	state =1;
	wake_up_interruptible(&key_wait);
	return;
}
static void gpio_keys_timer(unsigned long _data)
{
	//struct gpio_key_data *data = (struct gpio_key_data *)_data;
	//key_respone(data);

}

static irqreturn_t gpio_keys_isr(int irq, void *dev_id)
{
	struct gpio_key_data *bdata = dev_id;
	struct gpio_keys_button *button = bdata->button;
	BUG_ON(irq != gpio_to_irq(button->gpio));

	if (bdata->timer_debounce)
		mod_timer(&bdata->timer,
			jiffies + msecs_to_jiffies(bdata->timer_debounce));
	else
			if(able)
				key_respone(bdata);
	return IRQ_HANDLED;
}

static int __devinit gpio_keys_setup_key(struct platform_device *pdev,
					 struct gpio_key_data *bdata,
					 struct gpio_keys_button *button)
{
	char *desc = button->desc ? button->desc : "gpio_keys";
	struct device *dev = &pdev->dev;
	unsigned long irqflags;
	int irq, error;

	setup_timer(&bdata->timer, gpio_keys_timer, (unsigned long)bdata);

	error = gpio_request(button->gpio, desc);
	if (error < 0) {
		dev_err(dev, "failed to request GPIO %d, error %d\n",
			button->gpio, error);
		goto fail2;
	}
	
	error = gpio_direction_input(button->gpio);
	if (error < 0) {
		dev_err(dev, "failed to configure"
			" direction for GPIO %d, error %d\n",
			button->gpio, error);
		goto fail3;
	}
		
	if (button->debounce_interval) {
		error = gpio_set_debounce(button->gpio,
					  button->debounce_interval * 1000);
		/* use timer if gpiolib doesn't provide debounce */
		if (error < 0)
			bdata->timer_debounce = button->debounce_interval;
	}

	irq = gpio_to_irq(button->gpio);
	if (irq < 0) {
		error = irq;
		dev_err(dev, "Unable to get irq number for GPIO %d, error %d\n",
			button->gpio, error);
		goto fail3;
	}

	irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
	/*
	 * If platform has specified that the button can be disabled,
	 * we don't want it to share the interrupt line.
	 */

	error = request_irq(irq, gpio_keys_isr, irqflags, desc, bdata);
	if (error) {
		dev_err(dev, "Unable to claim irq %d; error %d\n",
			irq, error);
		goto fail3;
	}
	gpio_export(button->gpio, false);
	return 0;

fail3:
	gpio_free(button->gpio);
fail2:
	return error;
}
static ssize_t keys_open(struct inode  * inode, struct file * file)
{
	able =1;/*instead  of enable irq*/
	return 0;
}

static ssize_t keys_read(struct file * file, char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	if(!(file->f_flags & O_NONBLOCK))
		{
			wait_event_interruptible(key_wait, state );
		}
		if(get_my_key.value == 0 && state == 1 && get_my_key.code != 116)
			buzzer_beep();
	ret =copy_to_user(buf, &get_my_key, count);	
	state =0;
	return count;
}


static long key_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
	//void __user *uarg = (void __user *) arg;
	int err;
	unsigned int *gpio_2_ts;
	gpio_2_ts = (u32*)ioremap(GPIO_1,SZ_8K);
	
	switch (cmd) {
	case 	TS_UNABLE:
		gpio_reg_write(gpio_2_ts,GPIO_IRQENABLE_CLR_0,1<<27);
		break;
	case TS_ENABLE:
		gpio_reg_write(gpio_2_ts,GPIO_IRQENABLE_SET_0,1<<27);
		break;
	default:
		pr_info("CMD err! TS_UNABLE  or TS_ENABLE");
		err = -1;
		break;
		}	
	
	iounmap(gpio_2_ts);

		
	return err;
}

int keys_release(struct inode *inode, struct file *file)
{
	able = 0;
	return 0;
}


static struct file_operations keys_openration ={
	.owner = THIS_MODULE,
	.open = keys_open,
	.read = keys_read,
	.unlocked_ioctl	= key_ioctl,
	.release = keys_release,
};

static int __devinit keys_probe(struct platform_device *pdev)
{
	struct keys_platform_data *pdata = pdev->dev.platform_data;
	struct gpio_keys_datadrv *ddata;
	struct device *dev = &pdev->dev;
	int i, error;
	
	error = buzzer_beep_init();
	if(error)
		{
		pr_info("buzzer_beep_init error\n");
		return error;
		}
	major=register_chrdev( 0, "gpio_keys",&keys_openration);

	if(!major)
		{
			pr_info("chrdev register error\n");
			goto fail1;
		}
	
	keys_class = class_create(THIS_MODULE, "keys_class");
		if (IS_ERR(keys_class)) {
		error = PTR_ERR(keys_class);
		goto fail2;
	}
		
	device_create(keys_class, NULL, MKDEV(major, 0), NULL, "gpio_keys"); 

/*zz-8-9-L2*/
#if 1
	key_mux = ioremap(KEY_MUX,2);
	if(!key_mux)
		{
			pr_info("key mux error\n");
				goto fail3;
		}
	__raw_writel(0x00050080,key_mux);
#endif
	ddata = kzalloc(sizeof(struct gpio_keys_datadrv) +
			pdata->nbuttons * sizeof(struct gpio_key_data),
			GFP_KERNEL);
	if (!ddata) {
		dev_err(dev, "failed to allocate state\n");
		error = -12;
		goto fail4;
	}
	ddata->n_buttons = pdata->nbuttons;
	mutex_init(&ddata->disable_lock);
	platform_set_drvdata(pdev, ddata);

	/* Enable auto repeat feature of Linux input subsystem */

	for (i = 0; i < pdata->nbuttons; i++) {
		struct gpio_keys_button *button = &pdata->buttons[i];
		struct gpio_key_data *bdata = &ddata->data[i];
		bdata->button = button;

		error = gpio_keys_setup_key(pdev, bdata, button);
		if (error)
			goto fail5;
	}
		init_waitqueue_head(&key_wait);
	return 0;

 fail5:
	while (--i >= 0) {
		free_irq(gpio_to_irq(pdata->buttons[i].gpio), &ddata->data[i]);
		if (ddata->data[i].timer_debounce)
			del_timer_sync(&ddata->data[i].timer);
		gpio_free(pdata->buttons[i].gpio);
	}
	platform_set_drvdata(pdev, NULL);
fail4:	
	kfree(ddata);
fail3:
	iounmap(key_mux);
fail2:
	device_destroy(keys_class,  MKDEV(major, 0));
	class_destroy(keys_class);
	unregister_chrdev(major, "gpio_keys");
 fail1:
	buzzer_beep_detory();

	return error;
}

static int __devexit keys_remove(struct platform_device *pdev)
{
	struct keys_platform_data *pdata = pdev->dev.platform_data;
	struct gpio_keys_datadrv*ddata = platform_get_drvdata(pdev);
	int i;
	
	for (i = 0; i < pdata->nbuttons; i++) {
		int irq = gpio_to_irq(pdata->buttons[i].gpio);
		free_irq(irq, &ddata->data[i]);
		if (ddata->data[i].timer_debounce)
			del_timer_sync(&ddata->data[i].timer);
		gpio_free(pdata->buttons[i].gpio);
	}

		kfree(ddata);
		iounmap(key_mux);
		device_destroy(keys_class,  MKDEV(major, 0));
		class_destroy(keys_class);
		unregister_chrdev(major, "gpio_keys");
		buzzer_beep_detory();
		pr_info("\"dev/gpio_keys\" rmoved ok!\n");

	return 0;
}

static struct platform_driver my_gpio_keys = {
	.probe		= keys_probe,
	.remove		= __devexit_p(keys_remove),
	.driver		= {
		.name	= "gpio-keys",
		.owner	= THIS_MODULE,
	}
};

static int __init gpio_keys_init(void)
{
	return platform_driver_register(&my_gpio_keys);
}

static void __exit gpio_keys_exit(void)
{	
	platform_driver_unregister(&my_gpio_keys);
}

module_init(gpio_keys_init);
module_exit(gpio_keys_exit);
MODULE_LICENSE("GPL");

