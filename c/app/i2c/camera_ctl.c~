/*************************************************************************
> File Name: camera_ctl.c
> Author: Richard
> Mail: freedom_wings@foxmail.com 
> Created Time: 2014年04月19日 星期六 15时43分56秒
************************************************************************/

#include "camera_ctl.h"

static int camera_fd = -1;
static int camera_mode = -1;
static int pcie_gpio_fd = -1;

static int i2c_read(unsigned char address)
{
    int ret;
    char read_buff[2];


    ret = write(camera_fd,&address,1);
    if(ret != 1)
    {
        perror("i2c_read send read address error");
        return -1;
    }
    ret = read(camera_fd,read_buff,2);
    if(ret != 2)
    {
        perror("i2c_read read error");
        return -1;
    }

    return ((((int)read_buff[0]) << 8) | (read_buff[1] & 0xff));

}


static int  i2c_write(unsigned short d_buff,unsigned char address)
{
    int ret;
    char send_buff[3];

    send_buff[0] = address;
    send_buff[1] = d_buff >> 8;
    send_buff[2] = d_buff & 0xff;

    ret = write(camera_fd,&send_buff,3);
    if(ret != 3)
    {
        perror("i2c_write write error");
        return -1;
    }

    return 0;
}

int set_trigger_mode(int mode)
{
    int flag = mode;
    int ret;
    switch(flag) 
    {


        case SOFTWARE_TRIGGER_MODE:  
        i2c_write(0x00,GET_MODE_SETREG);
        i2c_write(SOFTWARE_TRIGGER_MODE,GET_MODE_SETREG);
        ret = i2c_read(GET_MODE_SETREG);
        #if defined(DEBUG)
        printf("set SOFTWARE_TRIGGER_MODE address 0x%0x:%d\n",GET_MODE_SETREG, \
               i2c_read(GET_MODE_SETREG));
        #endif
        camera_mode = SOFTWARE_TRIGGER_MODE;
        break;
        case IO_TRIGGER_MODE:  
        printf("unused mode\n");
        ret = -1;
        break;
        case VIDEO_TRIGGER_MODE:  
        i2c_write(0x00,GET_MODE_SETREG);
        i2c_write(VIDEO_TRIGGER_MODE,GET_MODE_SETREG);
        ret = i2c_read(GET_MODE_SETREG);
        #if defined(DEBUG)
        printf("set VIDEO_TRIGGER_MODE address 0x%0x:%d\n",GET_MODE_SETREG, \
               i2c_read(GET_MODE_SETREG));
        #endif
        camera_mode = VIDEO_TRIGGER_MODE;
        break;
        default:				printf("unknow trigger mode\n");
        ret = -1;
        break;

        return ret;
    }
}


int set_exposure_time(int nshutter) // time is us 
{
    int ntime;
    int ret;
    unsigned short low;
    unsigned short high;
    ntime = nshutter * 36;
    low = ntime;
    printf("low =%hu\n",low);
    ret = i2c_write(low,LOW_EXPOSURE_SETREG);
    if(ret < 0)
    {
        perror("set exposure_time low reg err");
        return -1;
    }
    #if defined(DEBUG)
    printf("set LOW_EXPOSURE_SETREG address 0x%0x:%d\n",LOW_EXPOSURE_SETREG, \
           i2c_read(LOW_EXPOSURE_SETREG));
    #endif

    high = ntime >> 16;
    printf("high =%hd\n",high);
    ret = i2c_write(high,HIGH_EXPOSURE_SETREG);
    if(ret < 0)
    {
        perror("set exposure_time high reg err");
        return -1;
    }
    #if defined(DEBUG)
    printf("set HIGH_EXPOSURE_SETREG address 0x%0x:%d\n",HIGH_EXPOSURE_SETREG, \
           i2c_read(HIGH_EXPOSURE_SETREG));
    #endif

    ret = i2c_read(LOW_EXPOSURE_SETREG);
    ret |= i2c_read(HIGH_EXPOSURE_SETREG)<< 16;
    #if defined(DEBUG)
    printf("set exposure time %d\n",ret);
    #endif
    return ret/36;
}

int camera_capture(void)
{
    int flag = camera_mode;
    int ret;
    switch(flag) 
    {
        case IO_TRIGGER_MODE:  			
        printf("unknow  get_capture mode\n");
        ret = -1;
        break;
        case SOFTWARE_TRIGGER_MODE:  
        i2c_write(0x00,GET_MODE_SETREG);
        i2c_write(0x01,GET_MODE_SETREG);
        #if defined(DEBUG)
        printf("set SOFTWARE_TRIGGER_MODE address 0x%0x:%d\n",GET_MODE_SETREG, \
               i2c_read(GET_MODE_SETREG));
        #endif
        break;
        case VIDEO_TRIGGER_MODE:  
        printf("unknow  get_capture mode\n");
        ret = -1;
        break;
        default:		printf("unknow  get_capture mode maybe camera_mode didn't set\n");									
        ret = -1;
        break;

    }
}

int enable_flashlight(int mode)
{
    int flag = mode;
    int ret = 0;
    if(mode >=0 && mode < 4)
    {
        i2c_write(mode && 0x01,FLASH_LIGHT_CTL_1);
        i2c_write(mode && 0x02,FLASH_LIGHT_CTL_2);
        if(i2c_read(FLASH_LIGHT_CTL_1) != (mode & 0x01))
        {
            printf("set flashlight ctrl 1 %s err\n",(mode && 0x01)?"on":"off");
            ret = -2;
        }
        if(i2c_read(FLASH_LIGHT_CTL_2) != (mode & 0x2))
        {
            printf("set flashlight ctrl 2 %s err\n",(mode && 0x02)?"on":"off");
            ret = -3;
        }
        #if defined(DEBUG)
        printf("set flashlight A_MODE ctrl 1:%d,ctrl 2:%d\n",i2c_read(FLASH_LIGHT_CTL_1),i2c_read(FLASH_LIGHT_CTL_2));
        #endif

        return ret;
    }
    else
    printf("err flashlight mode:%d\n",mode);
    return -1;
}

int set_sharpen(int mode)
{
    int flag = mode;
    int ret;
    if(mode > 0 && mode < 9)
    {
        i2c_write(mode,SHARPEN_CAPTURE_SET);
        ret = i2c_read(SHARPEN_CAPTURE_SET);

        if(ret != mode)
        return -1;
        #if defined(DEBUG)
        printf("set SHARPEN_CAPTURE %d\n",i2c_read(SHARPEN_CAPTURE_SET));
        #endif
    }
    else
    {
        printf("sharpen mode support 1~8\n");
        return -1;
    }
}

int set_contrast(int mode)
{
    int flag = mode;
    int ret;
    if(mode > 0 && mode < 16)
    {
        i2c_write(mode,CONTRAST_SET);
        ret = i2c_read(CONTRAST_SET);
        #if defined(DEBUG)
        printf("set CONTRAST %d\n",i2c_read(SHARPEN_CAPTURE_SET));
        #endif
    }
    else
    {
        printf("contrast mode support 1~15\n");
        return -1;
    }
}


void gpio_set(int pin,int value)
{
    char system_cmd[50];
    int gpio_pin = pin;
    int gpio_value = value;

    if((gpio_pin == CPU_GPIO1) ||(gpio_pin == CPU_GPIO2) || (gpio_pin == CPU_GPIO3) ||(gpio_pin == CPU_RESET))
    {
        sprintf(system_cmd,"echo %d > /sys/class/gpio/gpio%d/value",gpio_value,gpio_pin);
        #if defined(DEBUG)
 //       printf("run command:%s\n",system_cmd);
        #endif
        system(system_cmd);
    }
    else
    printf("unknow gpio pin %d\n",gpio_pin);
}



static int n_gain04 = 0,n_gain05 = 0;
static int m_nbluegain = 0,m_nredgain = 0,m_ngreen1gain = 0,m_ngreen2gain = 0;

typedef struct s_gpio_set {
    int pin;
    int value;
}gpio_set_s;

static void pcie_gpio_func(int pin, int value)
{
    gpio_set_s pcie_gpio_w;
    pcie_gpio_w.pin = pin;
    pcie_gpio_w.value = value;
    write(pcie_gpio_fd,&pcie_gpio_w,sizeof(pcie_gpio_w));    
}

static void setgpio_1k_low(void) 
{ 
  pcie_gpio_func(CPU_GPIO1, 0); 
  return; 
} 

static void setgpio_1k_high(void) 
{ 
  pcie_gpio_func(CPU_GPIO1, 1); 
  return; 
}

static void setgpio_3l_low(void) 
{ 
  pcie_gpio_func(CPU_GPIO3, 0); 
  return; 
} 

static void setgpio_3l_high(void) 
{ 
  pcie_gpio_func(CPU_GPIO3, 1); 
  return; 
} 

static int write_a(unsigned char cdata)   
{ 
  int i = 0; 
  for (i = 0;i < 8;i++) 
  { 
    setgpio_1k_low(); 
   pcie_gpio_func(CPU_GPIO2, cdata&0x01); 
    setgpio_1k_high(); 
    cdata>>=1; 
  } 
  setgpio_1k_low(); 
  return 0; 
} 
 
static int write_d(int ndata)   
{ 
  int i = 0; 
  for (i = 0;i < 24;i++) 
  { 
      setgpio_1k_low(); 
      pcie_gpio_func(CPU_GPIO2, (ndata & 0x0001)); 
    setgpio_1k_high(); 
    ndata >>= 1; 
  } 
  setgpio_1k_low(); 
  return 0; 
} 



static int write_value(unsigned char lowaddr, int nvalue)   
{ 
  setgpio_3l_low();   
 // sys_wait(100);    //延时 100us 
 usleep(100);
  write_a(lowaddr); 
  write_d(nvalue); 
//  sys_wait(100);    //延时 100us 
  usleep(100);
  setgpio_3l_high();   
  return 0; 
} 

 int set_green1_gain(int ngain)        
{ 
  n_gain04 = n_gain04 & 0xfffffe00 | ngain; 
  write_value(0x04, n_gain04); 
  m_ngreen1gain = ngain; 
  return m_ngreen1gain; 
}

int get_green1_gain(void)     
{ 
  return m_ngreen1gain; 
} 
 
int set_green2_gain(int ngain)     
{ 
  n_gain05 = n_gain05& 0x1ff | (ngain<<9);   
  write_value(0x05, n_gain05);    
  m_ngreen2gain = ngain;   
  return m_ngreen2gain; 
} 
 
int get_green2_gain(void)   
{ 
  return m_ngreen2gain; 
} 
 
int set_red_gain(int ngain)      
{ 
  n_gain04 = n_gain04 & 0x1ff | (ngain<<9); 
  write_value(0x04, n_gain04); 
  m_nredgain = ngain; 
  return m_nredgain; 
} 
 
int get_red_gain(void)    
{ 
  return m_nredgain; 
} 

int setbluegain(int ngain)     
{ 
  n_gain05 = n_gain05 & 0xfffffe00 | ngain; 
  write_value(0x05, n_gain05); 
  m_nbluegain = ngain; 
  return m_nbluegain; 
} 


int get_blue_gain(void)    
{ 
  return m_nbluegain; 
}

int set_global_gain(int ngain)     
{ 
 
  write_value(0x01, ngain); 
  return 0; 
} 


static int init_device(void)      
{         
       WriteValue(0x10,0x00000001); 
  WriteValue(0x12,0x00000000); 
  WriteValue(0x12,0x00000001);     
  WriteValue(0x14,0x00000001); 
  WriteValue(0x40,0x00000001); 
        WriteValue(0x42,0x00000001); 
  WriteValue(0x00,0x00000004);     
      WriteValue(0x17,0x00000001); 
WriteValue(0x1a,0x00000001); 
WriteValue(0x18,0x00000003); 
  WriteValue(0x02,0x00000008); 
        WriteValue(0x03,0x00000005); 
        WriteValue(0x20,0xF);     
        WriteValue(0x21,0x00765750);       
        WriteValue(0x22,0x007360F0);   
        WriteValue(0x25,6); //这个后面写入的值是十进制的 
  WriteValue(0x26,3); //这个后面写入的值是十进制的 
        WriteValue(0x27,10); //这个后面写入的值是十进制的 
        WriteValue(0x30,0x0); 
        WriteValue(0x31,0x007360F0);     
        WriteValue(0x60,0x00001001);     
        WriteValue(0x61,0x00000801);   
  WriteValue(0x62,0x00006DB6); 
  WriteValue(0x63,0x00000124); 
  WriteValue(0x64,0x00000000); 
  WriteValue(0x11,0x00000001);   
  WriteValue(0x14,0x00000000);     
      return 0; 
} 


int init_camera_i2c(void)
{

    int ret;
    gpio_set(CPU_RESET,1);
//    gpio_set(CPU_GPIO1,1);
  //  gpio_set(CPU_GPIO2,1);
   // gpio_set(CPU_GPIO3,1);
    camera_fd = open(I2C_DEV,O_RDWR);
    if(camera_fd < 0)
    {
        perror("open i2c dev failed");
        return -1;
    }

    pcie_gpio_fd = open(PCIE_DEV,O_RDWR);
    if(pcie_gpio_fd < 0)
    {
        perror("open pcie_gpio dev failed");
        return -1;
    }

    ret = ioctl(camera_fd,I2C_SLAVE,SLAVE_ADDR);
    if(ret < 0)
    {
        perror("set i2c slave address failed");
        return -1;
    }
    /*
    * first test i2c
    */
    ret = i2c_read(TEST_REG);

    if(ret != 0x8448)
    {
        perror("i2c test read err");
        return -1;
    }
    
	init_device();
	
    return 0;
}

#if 1
int main(void)
{
    int ret;
    int cmd;
    int pin,value;
    int mode,time;
    printf(" init_camera_i2c test\n");
    ret = init_camera_i2c();
    if(ret < 0)
    {
        perror(" init_camera_i2c failed");
        return -1;
    }
    while(1)
    {
        printf("\n \
               1,gpio_set\n \
               2,set_contrast\n \
               3,set_sharpen\n \
               4,enable_flashlight\n \
               5,camera_capture\n \
               6,set_exposure_time\n \
               7,set_trigger_mode\n: ");

        scanf("%d",&cmd);

        switch(cmd)
        {
            case 1:
            while(1)
            {					
                printf("gpio pin and value:");
                scanf("%d %d",&pin,&value);
                gpio_set(pin,value);
                while(getchar()!='\n');
                printf("input q to exit,other continue:");
                if(getchar() == 'q')
                break;
            }
            break;
            case 2:
            while(1)
            {	
                printf("contrast mode:");
                scanf("%d",&mode);
                set_contrast(mode);
                while(getchar()!='\n');
                printf("input q to exit,other continue:");
                if(getchar() == 'q')
                break;
            }
            break;
            case 3:
            while(1)
            {	
                printf("sharpen mode:");
                scanf("%d",&mode);
                set_sharpen(mode);
                while(getchar()!='\n');
                printf("input q to exit,other continue:");
                if(getchar() == 'q')
                break;
            }
            break;
            case 4:
            while(1)
            {	
                printf("flashlight mode:");
                scanf("%d",&mode);
                enable_flashlight(mode);
                while(getchar()!='\n');
                printf("input q to exit,other continue:");
                if(getchar() == 'q')
                break;
            }				
            break;
            case 5:
            while(1)
{	
    camera_capture();
    while(getchar()!='\n');
    printf("input q to exit,other continue:");
    if(getchar() == 'q')
    break;
}				

                break;
            case 6:
                while(1)
                {	
                    printf("set_exposure_time mode:");
                    scanf("%d",&time);
                    set_exposure_time(time);
                    while(getchar()!='\n');
                    printf("input q to exit,other continue:");
                    if(getchar() == 'q')
                    break;
                }				

                break;
            case 7:
                while(1)
                {	
                    printf("set_trigger_mode mode:");
                    scanf("%d",&mode);
                    set_trigger_mode(mode);
                    while(getchar()!='\n');
                    printf("input q to exit,other continue:");
                    if(getchar() == 'q')
                    break;
                }				

                break;
            default :break;
            }
            }
            return 0;
}

#endif

