#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/fs.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<asm/uaccess.h>
#include<asm/io.h>

//�ַ��豸�������Ϳ��

static unsigned int major=0;//�������豸��

static struct cdev *led_cdev=NULL;//����һ���ַ��豸�����ָ��


static volatile unsigned long *gpfcon;//��������ӿڵ�ָ��
static volatile unsigned long *gpfdat;


//����udev����ʵ���豸�ڵ���Զ�����

static struct class *led_class;
static struct class_device *led_class_device;
//static unsigned long cnt=0;
static unsigned int ker_i=100;


static ssize_t  led_write(struct file *file, const char __user *data,size_t len, loff_t *ppos)
{

	int ret;

	if(len!=4)
		return -EINVAL;

	//ret=0��ȫ����
	ret=copy_from_user(&ker_i, data, len);

	if(ker_i!=0)
	
		*gpfdat &=~(1<<(ker_i+3));//LED��
	else
		*gpfdat |=0xf0;		//LED��
	
	
	
	
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
	printk("major=%d,minor=%d\n",major,minor);//�����豸��

	//ledӲ����ʼ��
	*gpfcon &=~((3<<8)|(3<<10)|(3<<12)|(3<<14));
	*gpfcon |=((1<<8)|(1<<10)|(1<<12)|(1<<14));


	printk("call %s\n",__FUNCTION__);
	return 0;
}


//�ַ��豸�����зǳ��ؼ���һ�����ݽṹ
static struct file_operations led_fops = {
	.owner	= THIS_MODULE, 	//ָ������ģ��
	.write	=led_write,	//ָ���豸д����
	.read	=led_read,	//ָ���豸������
	.open	= led_open	//ָ���豸�򿪺���
};

//ģ��ĳ�ʼ������
static int __init led_init(void)
{
	int ret;
	dev_t devno;
	
	//����majorֵ��̬��̬�����豸��
	if(major){
		devno=MKDEV(major,0);
		ret=register_chrdev_region(devno,1, "led");//ָ��һ��δ�����豸��
		
			
	}else{
		ret=alloc_chrdev_region(&devno, 0,1,"led");//�Զ������豸��
		major=MAJOR(devno);
				
	}
	if(ret<0){
		printk("cannot alloc major=%d\n",major);
		return ret;
	}
	
	
	led_cdev=cdev_alloc();   		//����һ���ַ��豸����
	cdev_init(led_cdev,&led_fops);	//��ʼ���ַ��豸����
	
	led_cdev->owner=THIS_MODULE;
	ret=cdev_add(led_cdev,devno,1);//���ַ��豸����ע���ϵͳ
	
	if(ret<0){
		printk("cannot register chrdev\n");
		return ret;
	}
	
		
	led_class=class_create(THIS_MODULE,"led_class");
	led_class_device=device_create(led_class,NULL, devno,NULL,"led");

	//��һ��io��ַ�ռ�ӳ�䵽�ں˵������ַ�ռ�
	gpfcon=ioremap(0x56000050,8);
	gpfdat=gpfcon+1;
	
	printk("call %s:major=%d,minor=%d\n",__FUNCTION__,MAJOR(devno),MINOR(devno));		
		
	return 0;
}

static void __exit led_exit(void)
{
	
	dev_t devno=MKDEV(major,0);	
	unregister_chrdev_region(devno,1);//�ͷŷ�����豸��
	
	device_unregister(led_class_device);
	class_destroy(led_class);


	cdev_del(led_cdev);//���ַ��豸�����ϵͳ��ע����
	
	iounmap(gpfcon);
	printk("call %s\n",__FUNCTION__);

}



module_init(led_init);
module_exit(led_exit);


MODULE_AUTHOR("farsight");
MODULE_DESCRIPTION("led world!");

MODULE_LICENSE("GPL");








