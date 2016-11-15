#include  "Mini5D_BLDC_V16.h"
#include  "Mini5D_BLDC.h"               

uint32_t volatile Duty_Command ;                           // Duty命令, 范围 0 ~ CNR
uint32_t  volatile Pulse_Current ;                         // PPM 当前值
uint32_t  volatile PulseTick ;                             // PPM 上沿时刻 
uint32_t           Pulse_Min    = 1000 ;                   // PPM 最小值,缺省1000us
uint32_t           Pulse_Length = 1000 ;                   // PPM 最大值与最小值的差
////=====================================================================================             
////UART0 Initial ///////////////////////////////////////////////////////////////////////
void UART0_Init(uint32_t Baud)                  
{
  UART0->FUN_SEL = 0  ;                                    //UART 模式
  UART0->LCR     = 0x0007 ;                                //8bit, 2-stop, no parity
  UART0->BAUD    = 0x30000000 + 22118400/Baud - 2 ;        //UART 时钟 22M
  UART0->FCR     = 0x00020020 ;                            //RTS trigger:8, FIFO trigger:8 BYTE
  UART0->IER     = 0 ;                                   
}
/////////////////////////////////////////////////////////////////////////////////////////
// Configure P2.23456 & P0.4 as PWM function pin.  configure P1.0345 as ACMP input pin
void GPIO_Init(void) 
{                                          // P01配置成OUT模式,才能由PHCHG控制,QB模式不行      
  // P0.014567 六个, GPIO0->DOUT 的初值在函数PWM_ACMP0_T0_T1_Init()中写入
  GPIO0->PMD  = Px7_QB | Px6_QB | Px5_QB | Px1_OUT | Px0_OUT ;         // P0.1 可以为 A-
  GCR->P0_MFP = 0xF010 ;          // P0.4_PWM5, P0.0567_SPI, 就P04有用
                                
  //GPIO1->DOUT = 0 ;//
  GPIO1->PMD  = Px5_OUT | Px4_OUT |Px3_OUT | Px2_OUT | Px0_OUT ;     // P1.02345 五个
  GPIO1->OFFD = 0x00FF0000 ;                               //P1 数字输入通道全关闭
  GCR->P1_MFP = 0x303D ;                                   // P1.023_ADC123, P1.45_ACMP0
                           // GPIO2->DOUT 的初值在函数PWM_ACMP0_T0_T1_Init()中写
  GCR->P2_MFP = 0x7C00 ;                                   // P2.23456,五个,PWM01234
  
//引脚配置为输入时若什么都不接，引脚电压会上下漂，会增加漏电，也增加干扰
//配置为准双向输出1，就避免了电压上下漂。PCB就可以什么都不接了。
  GPIO3->DOUT = 0xFF ;                                     // P3.012456 六个
  GPIO3->OFFD = 0x00BF0000 ;                               // 只接通36的数字输入通道
  GPIO3->PMD  = Px6_QB  ;                            // P3.0145_CMP1,P3.6_GPIO, 
  GCR->P3_MFP = 0x01003376 ;                                  

  GPIO4->DOUT = 0xFF ;
  GPIO4->PMD  = Px7_QB | Px6_QB ;                          // P4.67,ICE接口烧录
  GCR->P4_MFP = 0 ;                                

  GPIO5->DOUT = 0xFF ;                                     // P5.012345 六个,(QFN33没有P5.5)
  GPIO5->PMD  = Px5_QB | Px4_QB | Px2_QB | Px1_QB | Px0_QB ;  //均未使用  
  GCR->P5_MFP = 0x0008 ;                                   // P5.3_ADC0

   //P36测PPM信号。
  GPIO3->ISR  = 0xFF ;                                     // Clear ISR  
  GPIO3->IMD  = 0 ;                                        // Edge interrupt mode
  GPIO3->IER  = 0x00400040 ;                               // 位16~23上沿使能,0~7下沿中断使能
  GPIO3->DBEN = 0x40 ;                                     // 1 mean De-bounce
  GPIODBNCE   = 0x2B ;                                             // De-bounce time = 2048HCLK

  NVIC_SetPriority(GPIO234_IRQn, 2);                       // Interrupt priority = 2 
  NVIC->ISER[0] = 1<<GPIO234_IRQn ;                                

  //这个没有使用
  GPIO5->ISR  = 0xFF ;                                     // Clear ISR  
  GPIO5->IMD  = 0 ;                                        // Edge interrupt mode
  GPIO5->IER  = 0x00000004 ;                               // 位16~23上沿使能,0~7下沿中断使能
  GPIO5->DBEN = 0x04 ;                                     // P5.2 Enable De-bounce

  NVIC_SetPriority(EINT1_IRQn, 2) ;                        //Interrupt priority = 2 
//  NVIC->ISER[0] = 1<<EINT1_IRQn ;                        // 按键
}
//=======================================================================================
// MAIN function                                                               
//---------------------------------------------------------------------------------------
int main(void) 
{  
  uint32_t  Adc_Vin, Adc_Tempera, temp32, LastDuty = ~0 ; 
    
  PWM_ACMP0_T0_T1_Init() ;      
  ADC->CR      = ADCR_EN ;                                 // 尽早使能ADC 
  ADC->ADSAMP  = 4 ;                                       // 采样时间 9 CLK
  ADC->ADTDCR  = 0 ;                                 //ADC Trigger Delay Controller Register 
                 //PWM频率: 5_22.1KHz, 6_18.4KHz, 7_15.8KHz, 8_13.8KHz, 10_11KHz, 14_7.9K
  PWM_one_percent = 8 ;                                                     
  PWM->CNR0       = 100*PWM_one_percent - 1 ;              // 设置 PWM 周期                  
  PWM->CNR1       = 100*PWM_one_percent - 1 ;                     
  Duty_Min        = 8*PWM_one_percent ;
    
  UART0_Init(115200) ; 
  GPIO_Init() ;          
  // 检测 BOD 电压是否配置为 2.7V 以下保持复位 //////////////////////////////////////////
  CLK->AHBCLK |= CLK_ISP_EN ; 
  FMC->ISPCON |= FMC_CFGUEN | FMC_EN_ ; 
  FMC->ISPADR  = 0x00300000 ;                              // Config0
  FMC->ISPCMD  = FMC_READ_0 ;                              // Read config0 
  FMC->ISPTRG  = 1 ;
  while(FMC->ISPTRG & 1) ; 
  temp32 = FMC->ISPDAT ;//读取配置值
  
  //假如BOD电压没有配置为2.7V，擦除后写入新的2.7V配置。
  if((temp32 & 0x00F004C1) != 0x00200481){           
    FMC->ISPCMD  = FMC_ERASE ;   
    FMC->ISPTRG  = 1 ;
    while(FMC->ISPTRG & 1) ;
    FMC->ISPDAT  = (temp32 & 0xFF0FFF3F) | 0x00200481 ;
    FMC->ISPCMD  = FMC_WRITE ;                             // 把 BOD 电压写成 2.7V 以下复位
    FMC->ISPTRG  = 1 ;
    while(FMC->ISPTRG & 1) ;
    GCR->IPRST_CTL1 = 1 ;                                  // Chip Reset 
  }

  //关闭寄存器改写
  FMC->ISPCON &= ~(FMC_CFGUEN | FMC_EN_) ; 
  CLK->AHBCLK &= ~CLK_ISP_EN ;  
  LOCKREG();  
  ///////////////////////////////////////////////////////////////////////////////////////
  gFlag     = 0 ;                                          // 清0标志
  ACMP_Flag = 0 ;                                          
  TIMER1->EXCON = TEX_DEBO_EN |TEX_EN | TEX_CAP_FALL_RISE; // 开始捕获PPM 脉冲
  Duty_Current  = 15*PWM_one_percent ;                      
  Motor_Beep(50000,900) ;  ++pPhase ;                      // AB 蜂鸣 50ms 
  Motor_Beep(50000,800) ;                                  // AC 蜂鸣

  //做油门的匹配
  if(Pulse_Current > 1600){
    Pulse_Length = Pulse_Current ; 
    while(Pulse_Current > 1400) ; 
    Delayus(500000) ;                                     
    Motor_Beep(50000,900) ;
    Pulse_Min    = Pulse_Current ;  
    Pulse_Length = Pulse_Length - Pulse_Min - 50 ; 
    Motor_Beep(50000,800) ;                               
  }
  Pulse_Length = ((100*PWM_one_percent)<<16)/Pulse_Length ;//左移16为了精度。
  Motor_Beep(50000,600) ;                                 

  //到此初始化完成，下面开始电机工作    
  ///////////////////////////////////////////////////////////////////////////////////////
  BLDC_STOP() ;                                                                                       
  ADC->CR |= ADCR_START_BUSY ;                             // 启动 ADC
  while(1){ 
    //// 测旋钮,温度,电压,电流 ========================================================== 
    temp32 = ADC->DR ;    
    if(temp32 & 0x20000){    // ADC数据有效,读清0,防真打开ADC寄存器窗时,可能会被防真器清0  
      temp32 &= 0x03FF ;
      switch(ADC->CHER & 0xFF){  
        case ADC2_VIN :
          Adc_Vin   = (Adc_Vin + temp32) >> 1 ;                                                    

          ADC->CHER = ADC3_TEMP ;                                      
          break ;   
        case ADC3_TEMP :
          Adc_Tempera = (Adc_Tempera + temp32) >> 1 ;  
                              
        default :                                          
          ADC->CHER = ADC2_VIN ;              
      } 
      //软件触发ADC 转换，也可以用PWM触发
      ADC->CR |= ADCR_START_BUSY ;     
    }
    ////=================================================================================
    if(gFlag & Flag_PPM_OK){                               // 有PPM信号, 更新占空比
      if(Pulse_Current <= Pulse_Min) Duty_Command = 0 ; 
      //Pulse_Length refer to Line 128
      else Duty_Command=((Pulse_Current - Pulse_Min)*Pulse_Length)>>16; // 变到 0 ~ CNR0左右   
      Duty_Target = Duty_Command ;   
    }
    
    temp32 = PulseTick ; 
    if(((TIMER1->DR - temp32)&0xFFFFFF) > 3000*1000){      // 3 秒无 PPM 调速信号
      PulseTick   = TIMER1->DR ; 
      Duty_Target = (Duty_Target*7) >> 3 ;                 // 目标占空比降为 87.5%
      gFlag      &= ~Flag_PPM_OK ;   
    }
    
    Duty_Target = 15*PWM_one_percent ;                     // 用于测试
    
    if((LastDuty ==0)&&(Duty_Target)){                     //快停转时 突然重新加电
      if(PeriodNow > 1500) BLDC_Start(PeriodMax, 0);       // 快速调整速度
      if(Duty_Current < Duty_Min) Duty_Current = Duty_Min ;    //占空比先拉到Duty_Min                              
    }
    LastDuty = Duty_Target ;   
    
    BLDC_Control() ;                           // BLDC 静止启动及调控  
    if(PeriodNow < (10000000ul/250000ul)) BLDC_STOP();     // 已超过250Krpm ? 停转        
    
    ////===== 其它任务放此 ==============================================================
  } 
}

void GPIO234_IRQHandler(void)                              // 调速脉宽测量  
{                                                          
  uint32_t temp32 ;                                    
  //When TEXIF flag is set to 1, the current TDR value will be auto-loaded into this 
  // TCAP filed immediately.
  // 捕获config 见Line 109
  // Pulse_Current range from 1000 t0 2000, PPM spec.
  
//  if(GPIO3->ISR & 0x40){                                 // GPIO234无其它中断,此句略
    if(GPIO3->PIN & 0x40) PulseTick = TIMER1->CAP ;        // PPM 上沿时刻
    else{//下降沿时刻
      temp32 = ((TIMER1->CAP - PulseTick)&0xFFFFFF) ;      // PPM 宽度,微秒数
      if(temp32 < 2500){Pulse_Current = temp32; gFlag |= Flag_PPM_OK; }          
    }
    TIMER1->EXISR = ~0 ;                                   // 清中断标志
//  }       //  End of "if(GPIO3->ISR & 0x40)"
  GPIO3->ISR = ~0 ;                                        // 清中断标志
}

//单纯用来打印信息
void SysTick_Handler(void)                                 // 20ms 中断一次
{   
  ++TickRelay ; 
#if 1                                                      // 打印速度信息
  {
    uint32_t static Cnt = 0, flag = 0 ;
    uint8_t  str[] = "Duty=   %         rpm\n\r" ; 
      
    if(++Cnt >= 25){  Cnt = 0 ; flag ^= 1 ;                // 20ms X 25 =0.5S
      if(flag == 0){  
        HexToStr(10000000/PeriodNow, &str[16]) ;           // 转速转换成字符
        Tx0FillFiFo(&str[10],13) ;                         // 写入 Tx0
      }
      else{  
        HexToStr(PWM->CMR0/PWM_one_percent, &str[7]) ;     // 占空比转换成字符
        Tx0FillFiFo(str,10) ;                              // 写入 Tx0
      }
    }      
  }
#endif  
}
