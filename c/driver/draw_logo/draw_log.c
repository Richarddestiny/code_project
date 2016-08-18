#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>
#include <bmp_log.h>

#define LCD_WIDTH	(800)
#define LCD_HIGHT 	(600)
#define FRAMBUFFER	(0x96D00000)
#define FRAMBUFFER_SIZE 	(LCD_WIDTH*LCD_HIGHT*3)
#define LOGO_SIZE  		 260
#define RING_RADIUS 		184
#define RING_WIDTH 		 100
#define X_COORDINATE 		  400
#define Y_COORDINATE 		300
static u32 *framebuffer_addr = NULL;

static int __init lcd_init(void)
{	
	unsigned int reg;
	int i,j;
	int k,l;

	framebuffer_addr = (u32*)ioremap(VPDMA_LIST_ATTR,FRAMBUFFER_SIZE);
	
	for(i=0;i<LOGO_SIZE;i++)
    {
        for(j = 0;j < LOGO_SIZE;j++)
        {
            if()
        }
    }	
	reg = __raw_readl(lcd_reg);

	
	reg |= ((0x3<<24) | (1<<20));
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
