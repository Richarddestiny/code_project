#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/fs.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<asm/uaccess.h>
#include<asm/io.h>

//字符设备驱动典型框架

static unsigned int major=0;//定义主设备号

static struct cdev *led_cdev=NULL;//定义一个字符设备对象的指针


static volatile unsigned long *gpfcon;//定义操作接口的指针
static volatile unsigned long *gpfdat;


//利用udev机制实现设备节点的自动创建

static struct class *led_class;
static struct class_device *led_class_device;
//static unsigned long cnt=0;
static unsigned int ker_i=100;


static ssize_t  led_write(struct file *file, const char __user *data,size_t len, loff_t *ppos)
{

	int ret;

	if(len!=4)
		return -EINVAL;

	//ret=0完全拷贝
	ret=copy_from_user(&ker_i, data, len);

	if(ker_i!=0)
	
		*gpfdat &=~(1<<(ker_i+3));//LED亮
	else
		*gpfdat |=0xf0;		//LED灭
	
	
	
	
	//printk("call %s %ld times\n",__FUNCTION__,++cnt);
	return 0;
}



static ssize_t led_read(struct file *file, char __user *buf,
				size_t len, loff_t *ppos)
{

	int ret;
	ret=copy_to_user(buf, &ker_i, len);
	
	
	
	printk("call %s\n",__FUNCTION__);
	return 0;
}


static int led_open(struct inode *inode, struct file *file)
{

	
	int major=MAJOR(inode->i_rdev);
	int minor=MINOR(inode->i_rdev);
	printk("major=%d,minor=%d\n",major,minor);//测试设备号

	//led硬件初始化
	*gpfcon &=~((3<<8)|(3<<10)|(3<<12)|(3<<14));
	*gpfcon |=((1<<8)|(1<<10)|(1<<12)|(1<<14));


	printk("call %s\n",__FUNCTION__);
	return 0;
}


//字符设备驱动中非常关键的一个数据结构
static struct file_operations led_fops = {
	.owner	= THIS_MODULE, 	//指定所属模块
	.write	=led_write,	//指定设备写函数
	.read	=led_read,	//指定设备读函数
	.open	= led_open	//指定设备打开函数
};

//模块的初始化函数
static int __init led_init(void)
{
	int ret;
	dev_t devno;
	
	//根据major值静态或动态申请设备号
	if(major){
		devno=MKDEV(major,0);
		ret=register_chrdev_region(devno,1, "led");//指定一个未分配设备号
		
			
	}else{
		ret=alloc_chrdev_region(&devno, 0,1,"led");//自动分配设备号
		major=MAJOR(devno);
				
	}
	if(ret<0){
		printk("cannot alloc major=%d\n",major);
		return ret;
	}
	
	
	led_cdev=cdev_alloc();   		//分配一个字符设备对象
	cdev_init(led_cdev,&led_fops);	//初始化字符设备对象
	
	led_cdev->owner=THIS_MODULE;
	ret=cdev_add(led_cdev,devno,1);//将字符设备对象注册进系统
	
	if(ret<0){
		printk("cannot register chrdev\n");
		return ret;
	}
	
		
	led_class=class_create(THIS_MODULE,"led_class");
	led_class_device=device_create(led_class,NULL, devno,NULL,"led");

	//将一个io地址空间映射到内核的虚拟地址空间
	gpfcon=ioremap(0x56000050,8);
	gpfdat=gpfcon+1;
	
	printk("call %s:major=%d,minor=%d\n",__FUNCTION__,MAJOR(devno),MINOR(devno));		
		
	return 0;
}

static void __exit led_exit(void)
{
	
	dev_t devno=MKDEV(major,0);	
	unregister_chrdev_region(devno,1);//释放分配的设备号
	
	device_unregister(led_class_device);
	class_destroy(led_class);


	cdev_del(led_cdev);//将字符设备对象从系统中注销掉
	
	iounmap(gpfcon);
	printk("call %s\n",__FUNCTION__);

}



module_init(led_init);
module_exit(led_exit);


MODULE_AUTHOR("farsight");
MODULE_DESCRIPTION("led world!");

MODULE_LICENSE("GPL");








