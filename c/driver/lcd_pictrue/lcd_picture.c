#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>

#define LCD_ADDRESS (0x96d00000)

static u32 *lcd_reg =NULL;
static int __init lcd_init(void)
{	
	

	lcd_reg = (u32*)ioremap(LCD_ADDRESS,800*600*3);
    memset(lcd_reg,1,800*600*3);

	printk(KERN_INFO "lcd_reg =%d\n",lcd_reg);
	
	
	return 0;
}

static void __exit lcd_exit(void)
{
	iounmap(lcd_reg);	
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_AUTHOR("zz");
MODULE_LICENSE("GPL");
