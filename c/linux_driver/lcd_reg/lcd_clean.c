#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/delay.h>
#define VPDMA_LIST_ATTR (0x4810D000+0x008)
#define VPS_REG_BASE (0x48100000)
#define VPS_CLKC_RESET (0x0104)
static u32 *lcd_reg =NULL;
static int __init lcd_init(void)
{	
	unsigned int reg;
	

	lcd_reg = (u32*)ioremap(VPS_REG_BASE+VPS_CLKC_RESET,SZ_1K);

	reg = __raw_readl(lcd_reg);
	printk(KERN_INFO "lcd_reg =%x\n",reg);
	reg |= 1;
//	reg |= ((0x3<<24) | (1<<20));
//	reg |= (0x3<<24) ;
//	reg |=  (1<<20);
	__raw_writel(reg,lcd_reg);						
    udelay(10);
    reg &= ~1;
    __raw_writel(reg,lcd_reg);
	
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
