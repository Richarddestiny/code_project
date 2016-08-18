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
#define GPIO_0 0x48032000
#define GPIO_OE 0x134 	/*output enable register*/
#define GPIO_DATAOUT 0x13c
#define GPIO_CLEARDATAOUT 0x190	/*wirte 1 to clear dataout register*/
#define GPIO_SETDATAOUT 0x194		/*write 1 to set dataout register*/


#define CPU_GPIO1 25
#define CPU_GPIO2 24
#define CPU_GPIO3 27
#define CPU_RESET 8

static void pcie_gpio(int pin, int value);
static struct class *pcie_class;
static u32 *gpio_data =NULL;

typedef struct s_gpio_set
{
    int pin;
    int value;
} gpio_set;
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

static void pcie_gpio(int pin,int value)
{
    int gpio_0_pin = pin;
    int gpio_value = value;
    unsigned int ret;
    ret = gpio_reg_read(gpio_data,GPIO_DATAOUT);
    if(gpio_value == 0)
        ret = ret & (~(1<< gpio_0_pin));
    else
        ret |= 1<< gpio_0_pin;
    gpio_reg_write(gpio_data,GPIO_DATAOUT,ret);
}

static ssize_t pcie_open(struct inode * inode,struct file * file)
{
    printk("pcie gpio open\n");
    return 0;
}

static ssize_t  pcie_write(struct file * file, const char __user *buf, size_t count, loff_t *ppos)
{
    gpio_set gpio_state_w;
    unsigned long reg;
    if((reg = copy_from_user(&gpio_state_w,buf,count)))
        return -EFAULT;

    if(gpio_state_w.pin == CPU_GPIO1 || gpio_state_w.pin == CPU_GPIO2 ||gpio_state_w.pin == CPU_GPIO3 || gpio_state_w.pin == CPU_RESET)
    {
        pcie_gpio(gpio_state_w.pin,gpio_state_w.value);
    }
    else
    {
        printk("pcie_gpio write not support\n");
    }
    return 0;
}
static ssize_t pcie_read(struct file * file, char __user *buf, size_t count, loff_t *ppos)
{

    printk("unsupport read\n");

    return 0;
}
static struct file_operations pcie_openration =
{
    .owner = THIS_MODULE,
    .open = pcie_open,
    .write = pcie_write,
    .read = pcie_read,
};

static int major;
static int pcie_init(void)
{
    major=register_chrdev( 0, "pcie",&pcie_openration);
    pcie_class = class_create(THIS_MODULE, "pcie");
    device_create(pcie_class, NULL, MKDEV(major, 0), NULL, "gpio_pcie"); /* /dev/xyz */
    gpio_data = (u32*)ioremap(GPIO_0,SZ_8K);
    return 0;
}
static void pcie_exit(void)
{
    unregister_chrdev( major, "pcie");
    device_destroy(pcie_class, MKDEV(major, 0));
    class_destroy(pcie_class);
    iounmap(gpio_data);
}
module_init(pcie_init);
module_exit(pcie_exit);
MODULE_AUTHOR("zz");
MODULE_LICENSE("GPL");
