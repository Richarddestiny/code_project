
#define   Mini5xxDE_GLOBAL         
#include  "Mini5D_BLDC_V16.h"
#include  "Mini5D_BLDC.h"     

#define   ACMP_PH_A                CMP1_PIN_P32               
#define   ACMP_PH_B                CMP1_PIN_P34               
#define   ACMP_PH_C                CMP1_PIN_P35   
#define   DETEC_UP                 0x80                    
#define   PWM_COMPL                (0x4000ul)   
#define   PWM1_OUT_EN              0                       // PWM1 输出 A- 开关信号 
//#define   PWM1_OUT_EN              0x0200                // PWM1 总是输出 PWM

// P23_PWM1, P01_D6, P00_D7 都可用于输出 A- 开关信号 ////////////////////////////////////
#define   PHASE_AB        PWM1_OUT_EN | (0x0162ul)                
#define   PHASE_AC        PWM1_OUT_EN | (0x014Aul)                      
#define   PHASE_BC        PWM1_OUT_EN | (0x044Aul)                      
#define   PHASE_BA        PWM1_OUT_EN | (0x0468ul&(~0x40)) // P01_D6 输出 A- 开关信号                     
#define   PHASE_CA        PWM1_OUT_EN | (0x1068ul&(~0x40))          
#define   PHASE_CB        PWM1_OUT_EN | (0x1062ul)                     

#define   PHASE_AB_         PWM_COMPL | (0x0362ul)       
#define   PHASE_AC_         PWM_COMPL | (0x034Aul)            
#define   PHASE_BC_         PWM_COMPL | (0x0C4Aul)                          
#define   PHASE_BA_         PWM_COMPL | (0x0C68ul&(~0x40)) // P01_D6 输出 A- 开关信号                        
#define   PHASE_CA_         PWM_COMPL | (0x3068ul&(~0x40))                               
#define   PHASE_CB_         PWM_COMPL | (0x3062ul)                   
 
// 正转数组,/////////////////////////////////////////////////////////////////////////////
//此数组默认上桥臂高电平开启，下桥臂低电平开启
uint32_t TabPhase[25] = {                        
    T0_TRG_PWM_EN | PHASE_CB  | ACMP_PH_A | DETEC_UP ,   
  
    T0_TRG_PWM_EN | PHASE_AB  | ACMP_PH_C ,                // Up-PWM  基本数据
    T0_TRG_PWM_EN | PHASE_AC  | ACMP_PH_B | DETEC_UP ,   
    T0_TRG_PWM_EN | PHASE_BC  | ACMP_PH_A ,            
    T0_TRG_PWM_EN | PHASE_BA  | ACMP_PH_C | DETEC_UP , 
    T0_TRG_PWM_EN | PHASE_CA  | ACMP_PH_B ,             
    T0_TRG_PWM_EN | PHASE_CB  | ACMP_PH_A | DETEC_UP ,   
  
    T0_TRG_PWM_EN | PHASE_AB_ | ACMP_PH_C ,                // Up-PWM  互补数据
    T0_TRG_PWM_EN | PHASE_AC_ | ACMP_PH_B | DETEC_UP ,   
    T0_TRG_PWM_EN | PHASE_BC_ | ACMP_PH_A ,             
    T0_TRG_PWM_EN | PHASE_BA_ | ACMP_PH_C | DETEC_UP , 
    T0_TRG_PWM_EN | PHASE_CA_ | ACMP_PH_B ,             
    T0_TRG_PWM_EN | PHASE_CB_ | ACMP_PH_A | DETEC_UP , 
                                                 // 上下都 On-PWM 数据，上下轮换                                                       
    T0_TRG_PWM_EN | (0x086B)  | ACMP_PH_C ,                // A+ -> B-PWM  
    T0_TRG_PWM_EN | PHASE_AC  | ACMP_PH_B | DETEC_UP ,   
    T0_TRG_PWM_EN | (0x206E)  | ACMP_PH_A ,                // B+ -> C-PWM
    T0_TRG_PWM_EN | PHASE_BA  | ACMP_PH_C | DETEC_UP , 
    T0_TRG_PWM_EN | (0x027A)  | ACMP_PH_B ,                // C+ -> A-PWM
    T0_TRG_PWM_EN | PHASE_CB  | ACMP_PH_A | DETEC_UP ,   
                                                         
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x1162ul),              // Up-PWM 150度方波非互补数据
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0142ul),        
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x054Aul),           
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0448ul&(~0x40)),  
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x1468ul&(~0x40)),   
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x1060ul&(~0x40)),         
//                                                         
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x3362ul),            // Up-PWM 150度方波互补数据
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0342ul),         
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0F4Aul),        
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0C48ul&(~0x40)), 
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x3C68ul&(~0x40)), 
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x3060ul&(~0x40)),        
};    

// 反转数组 /////////////////////////////////////////////////////////////////////////////
//此数组默认上桥臂高电平开启，下桥臂低电平开启

uint32_t TabPhaseRev[25] = {                        
    T0_TRG_PWM_EN | PHASE_BC  | ACMP_PH_A | DETEC_UP ,  
  
    T0_TRG_PWM_EN | PHASE_AC  | ACMP_PH_B ,   
    T0_TRG_PWM_EN | PHASE_AB  | ACMP_PH_C | DETEC_UP ,     
    T0_TRG_PWM_EN | PHASE_CB  | ACMP_PH_A ,      
    T0_TRG_PWM_EN | PHASE_CA  | ACMP_PH_B | DETEC_UP ,         
    T0_TRG_PWM_EN | PHASE_BA  | ACMP_PH_C ,                    
    T0_TRG_PWM_EN | PHASE_BC  | ACMP_PH_A | DETEC_UP ,    
  
    T0_TRG_PWM_EN | PHASE_AC_ | ACMP_PH_B ,   
    T0_TRG_PWM_EN | PHASE_AB_ | ACMP_PH_C | DETEC_UP ,    
    T0_TRG_PWM_EN | PHASE_CB_ | ACMP_PH_A ,      
    T0_TRG_PWM_EN | PHASE_CA_ | ACMP_PH_B | DETEC_UP ,               
    T0_TRG_PWM_EN | PHASE_BA_ | ACMP_PH_C , 
    T0_TRG_PWM_EN | PHASE_BC_ | ACMP_PH_A | DETEC_UP ,  
  
    T0_TRG_PWM_EN | 0x206B    | ACMP_PH_B ,                      // A+ -> C-PWM  
    T0_TRG_PWM_EN | PHASE_AB  | ACMP_PH_C | DETEC_UP ,    
    T0_TRG_PWM_EN | 0x087A    | ACMP_PH_A ,                      // C+ -> B-PWM 
    T0_TRG_PWM_EN | PHASE_CA  | ACMP_PH_B | DETEC_UP ,         
    T0_TRG_PWM_EN | 0x026E    | ACMP_PH_C ,                      // B+ -> A-PWM 
    T0_TRG_PWM_EN | PHASE_BC  | ACMP_PH_A | DETEC_UP ,   
                                                         
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x054Aul),         
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0142ul),          
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x1162ul),        
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x1060ul&(~0x40)),        
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x1468ul&(~0x40)), 
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0448ul&(~0x40)),  
//                                                         
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0F4Aul),        
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0342ul),         
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x3362ul),        
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x3060ul&(~0x40)),    
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x3C68ul&(~0x40)), 
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0C48ul&(~0x40)),  
};    

uint32_t  Shut_All_MOS = 0x6A | PWM1_OUT_EN ;              // 写入PHCHG关断六路MOS的数据 
//=======================================================================================
void UnlockReg(void)      
{
  while((GCR->RegLockAddr & 1) == 0){                      // 防止被打断解锁不成功
    GCR->RegLockAddr = 0x59 ; 
    GCR->RegLockAddr = 0x16 ;    
    GCR->RegLockAddr = 0x88 ;            
  }
} 

//startup_MINI51.s中调用，比main() 先执行
void SystemInit(void)      
{
    UnlockReg();                                          
    CLK->CLKDIV  = ADC_CLK_DIV_8bit(3) ;                   // ADC 时钟三分频
    CLK->CLKSEL0 = CLKS0_HCLK_22M_ | CLKS0_TICK_HCLK_DIV2_ ; 
    CLK->CLKSEL1 = CLKS1_PWM23_HCLK | CLKS1_PWM01_HCLK | CLKS1_UART_22M | CLKS1_TMR1_HCLK 
                 | CLKS1_TMR0_HCLK  | CLKS1_ADC_22M    | CLKS1_WDT_10K_ | CLKS1_SPI_22M ;
    CLK->CLKSEL2 = CLKS2_PWM45_HCLK ;
    CLK->APBCLK  = CLK_ADC_EN | CLK_PWM45_EN | CLK_PWM23_EN | CLK_PWM01_EN | CLK_UART_EN           
                 | CLK_TMR1_EN | CLK_TMR0_EN | CLK_ACMP_EN  | CLK_SPI_EN   |CLK_WDT_EN ;  
  
  SysTick->VAL   = 20*11000 ;                                      
  SysTick->LOAD  = 20*11000 ;                              // Period = 20mS, CLK=22Mhz/2
  SCB->SHP[1]    = ~0 ;                                    // Interrupt priority = 3 
  SysTick->CTRL  = 0x0003 ;                                // IE & EN 
}

void Delayus(uint32_t Time)                                // 最大16777216(24位计数)
{
  uint32_t T1 = TIMER1->DR ; 
  while(((TIMER1->DR - T1) & 0xFFFFFF) < Time) ;          
}

void PWM024_0_MOS_On(void)                                 // 改PWM024输出0时上功率MOS导通          
{
  uint32_t n ;
  PWM->PCR ^= (PCR_INV_EN(0)|PCR_INV_EN(2)|PCR_INV_EN(4)) ;          
  for(n=0; n<25; ++n){TabPhase[n] ^= 0x15; TabPhaseRev[n] ^= 0x15; } 
  Shut_All_MOS ^= 0x15 ;                                            
}

void PWM135_1_MOS_On(void)                                 // 改PWM135输出1时下功率MOS导通   
{
  uint32_t n ;
  PWM->PCR ^= (PCR_INV_EN(1)|PCR_INV_EN(3)|PCR_INV_EN(5)) ;         
  for(n=0; n<25; ++n){TabPhase[n] ^= 0x6A; TabPhaseRev[n] ^= 0x6A; }   
  Shut_All_MOS ^= 0x6A ;
}

void PWM_ACMP0_T0_T1_Init(void) 
{//22M PCLK , DIV 22, T0 and T1 CLK 1M, 1us/ count
  //write TIMER0->CMPR cause timer0 cnts return to 0;
  TIMER0->CSR   =  T_CRST | T_MODE_PERIODIC  | T_TDR_EN + 21 ;  
  TIMER1->CSR   =  T_CRST | T_MODE_CONTINUOUS | T_TDR_EN + 21 ;  // 22 分频
  TIMER1->CSR  |=  T_CEN ;                                
  NVIC_SetPriority(TMR1_IRQn, 1) ;                          
  NVIC->ISER[0] = 1<<TMR1_IRQn ;                          

  ACMP->SR       = ~0 ;   //clr all flag
  ACMP->CR0      =  ACMP_CPP0_P15_0 | ACMP_EN ;                  // P1.5做比较器0正输入端
  ACMP->CR1      =  ACMP_EN ;
  //PWM->PHCHG 来改变比较器输入选择
  PWM->PHCHGMASK =  PHCHG_CTL_CMP1 | P01_OUT_D6 | P00_OUT_D7 ;  //
    
  PWM->PPR = PPR_DIV45(2)   | PPR_DIV23(2)   | PPR_DIV01(2) ;   // PWM clk divide by 2, 11M
  PWM->CSR = CSR_DIV1_CH(5) | CSR_DIV1_CH(4) | CSR_DIV1_CH(3)   // CSR, no divide
           | CSR_DIV1_CH(2) | CSR_DIV1_CH(1) | CSR_DIV1_CH(0) ;                    
  PWM->PCR = PCR_GROUP_MODE | PCR_CLR_COUNTER
     | PCR_CH_EN(5) | PCR_PERIOD_MODE(5) | PCR_INV_EN(1) | PCR_CH_EN(4) | PCR_PERIOD_MODE(4) 
     | PCR_CH_EN(3) | PCR_PERIOD_MODE(3) | PCR_INV_EN(3) | PCR_CH_EN(2) | PCR_PERIOD_MODE(2) 
     | PCR_CH_EN(1) | PCR_PERIOD_MODE(1) | PCR_INV_EN(5) | PCR_CH_EN(0) | PCR_PERIOD_MODE(0);
  PWM->CMR0  = 0 ;       //初始占空比为0                                 
  PWM->CMR1  = 0 ;         //初始占空比为0                             
//  PWM->PDZIR = 0x111111 ;                 // 死区时间 18个 PWM CLK,约1.6us

  PWM024_0_MOS_On() ;                 // PWM024 输出低时 MOS 导通,调用此函数让输出反相     
  PWM135_1_MOS_On() ;                 // PWM135 输出高时 MOS 导通,调用此函数让输出反相 
   
  if(Shut_All_MOS & 0x20) GPIO0->DOUT = 0xFD ;             // D5_P0.4 关断 MOS
  else GPIO0->DOUT = 0xED ;                                
  if(Shut_All_MOS & 0x40) GPIO0->DOUT |= 0x02 ;            // D6_P0.1 关断 MOS
  
  GPIO2->DOUT     = Shut_All_MOS << 2 ;                    
  
  PWM->PHCHGNXT   = Shut_All_MOS ;                         
  PWM->PHCHG      = Shut_All_MOS ;                  
  PWM->TRGCON0    = 0x0008 ;    //预设    Enable PWM Trigger ADC Function While Channel0’s Counter Matching 0                        
  PWM->PIIR       = ~0 ;  //clear all PWM  Interrupt Indication flag
//  PWM->PFBCON   = (Shut_All_MOS<<24) | BRK1_SOURCE_CPO0 | BRK1_EN; //BRK  cfg   
  PWM->PIER       = PIER_BRKE_IE + PIER_PIE(0);  //break int and ch0 int enable                                     
  PWM->INTACCUCTL = 0 ;   //累加中断寄存器，设0不累加                            
  PWM->POE        = 0x3F ;  //enable 6 channel output                             
  
  NVIC_SetPriority(FB_IRQn, 0) ;                        
  NVIC->ISER[0]  = 1 << FB_IRQn ;                  
  
  pPhaseEnd = &TabPhase[6] ;                               // 先按正转初始化这些值  
  pPhase    = &TabPhase[1] ;                   
}

void BLDC_STOP(void)                                       // 还电机自由
{                                             
  TIMER1->CSR  &= ~T_IE ;                                  // 关中断
  PWM->PHCHGNXT =  Shut_All_MOS ;                        
  PWM->PHCHG    =  Shut_All_MOS ;   
  PeriodNow     =  11000000 ;                              // 这个值计算出来的转速为0  
  Delayus(20) ;                                           
}

//=======================================================================================
void Motor_Beep(uint32_t Duration, uint32_t Period) //蜂鸣时长和半周期(相当于音调),微秒数
{
  uint32_t temp32 = TIMER1->DR ;  
    
  if(pPhase > (pPhaseEnd-3)) pPhase = pPhaseEnd-5 ;  //避免后面几行溢出边界。       
  PWM->CMR0 = Duty_Current ;                               // Duty_Current 的值决定响度
  do{
    PWM->PHCHG = *(pPhase+3) & (~T0_TRG_PWM_EN) ;    //DISABLE changed by T0
    Delayus(Period) ;  BLDC_STOP() ;
    PWM->PHCHG = *pPhase & (~T0_TRG_PWM_EN) ;          
    Delayus(Period) ; BLDC_STOP() ; 
  }while(((TIMER1->DR - temp32) & 0xFFFFFF) < (Duration<<1)) ; 
  Delayus(30000) ; 
}

//// 启转, 停转, 调输出电压 /////////////////////////////////////////////////////////////
void  BLDC_Control(void)          
{ 
  int32_t DutyInc = 0 ;                       
  static uint32_t  OutImg = 0, DutyTim = 6000;                            
  
  if((TIMER1->CSR & T_IE) == 0)
  {  // 没转，运转时开了中断     
    if(Duty_Target >= (Duty_Min + 2*PWM_one_percent)) //Duty_Target比Duty_Min 大
     {   
           //  StartOffset  = 12 ;                                        // 起转时上下轮换 PWM       
          PWM->CMR1    = Duty_Min ;    
          PWM->CMR0    = Duty_Min ;   Delayus(100);                  // 最多一个 PWM 周期后更新                               
          pPhaseEnd    = &TabPhase[6] ;                              // 使用数组TabPhase[1] ~ TabPhase[6]                   
          pPhase       = &TabPhase[1] ;                              // 第一相启转
          ACMP->CR1    =  ACMP_EN ;                          //启动时没有20mv的回差ACMP_HYST_EN
          //以最小占空比启动
          Duty_Current = Duty_Min  ;                                 // 当前占空比,可以按电源电压调节   

          // PeriodShrink = 32 ;                                      // 启转阶段, 换相时间不变
          // PeriodMax    = 30*1000 ;
          Wait_Zero(TabPhase[6], 10*1000) ;     //
          BLDC_Start(PeriodMax, 600) ;                               // 起转,参数为起转周期 
          // GPIO54 ^= 1;  重启指示    
    } 
  }  
  else
  {   // 在转  
    if(OutImg != PWM->PHCHG)
    { 
      OutImg = PWM->PHCHG ;              // 刚换相
      if(Duty_Current < Duty_Target)      ++Duty_Current ;              // 加快增压速度

      //启动早期，最大占空比不能超过35%
      if(StepCount < 12){                                       
        if(Duty_Current > 35*PWM_one_percent) Duty_Current -= 5*PWM_one_percent ;
                                         
      }
      else{                                                            
        if(StepCount > 300) ACMP->CR1 = ACMP_EN | ACMP_HYST_EN;  // 防干扰
        
        // 稳速代码放此, 用换相周期 PeriodNow 计算当前速度,算出占空比增量DutyInc                                
      } 
      
      if(OutImg & PWM_COMPL) DutyTim = 3000 ;       //互补信号时            
      else DutyTim = 6000 ;         
    }

    //加速时，每次增加计数值1，减速时直接一步到位。
    if(Duty_Target <= Duty_Current)     Duty_Current = Duty_Target;  // 非上下互补可一减到底    
    else DutyInc = 1;    
      
    if(Duty_Current > 70*PWM_one_percent) BLDC_Add_Duty(DutyInc, 600, 0) ;      
    else if(Duty_Current > 35*PWM_one_percent) BLDC_Add_Duty(DutyInc, 1200, 0) ;        
    else BLDC_Add_Duty(DutyInc, DutyTim, 0) ;       

   //更新占空比
    PWM->CMR0 = Duty_Current ;                                                            
    PWM->CMR1 = Duty_Current ;  
        
    if((TIMER1->CSR & T_IE) == 0)       Delayus(100000);               // 刚刚停转,等100ms          
  }
}

//// 把发送FiFo填满 /////////////////////////////////////////////////////////////////////
// 形参 :  *pStr, 字符指针
//          Cnt , 字符个数
// 返回 :   pStr, 剩余的首字符地址
//          0   ,  字符已输出完成
uint8_t* Tx0FillFiFo(uint8_t*pStr, uint32_t Cnt)
{
  if(Cnt){                                                   
    do{
      if( UART0->FSR & TX_FIFO_FULL ) break ;              //FiFo满了就退出循环        
      UART0->THR = *pStr++ ;                                  
    }while(--Cnt) ;                                             
  }
  if(Cnt) return  pStr ; 
  else    return  0    ;                               
}

////convert a integer into a string for printing /////////////////////////////////////////
//input : Number, 32bit integer
//       *pStr,  the last address saving character.  
//return: Sum of character converted.
uint32_t HexToStr(uint32_t Number, uint8_t*pStr)
{
  const uint8_t Asc[10]="0123456789" ;
  uint32_t temp32u, Cnt = 0 ;

  do{
    temp32u = Number ; 
    Number /= 10 ; 
    *pStr-- = Asc[temp32u - Number*10];                    // 先得到低位 
    ++Cnt ;
  }while(Number); 
  return Cnt ;
}

/////////////////////////////////////////////////////////////////////////////////////////
//参数一Duty目标值, 参数二延时后再加   
__weak void BLDC_Add_Duty(int32_t DutyInc, uint32_t Delay, uint32_t Offset) 
{
  static uint32_t  Tmr1_Tick = 0 ;                                             
    
  if(((TIMER1->DR - Tmr1_Tick) & 0xFFFFFF) >= Delay){                        
    Duty_Current += DutyInc ;                                                       
    Tmr1_Tick     = TIMER1->DR ;                            
  } 

  // 当油门关掉了，等速度掉下来后，才执行停机。
  //油门关掉，占空比直接到0，等电机自然减速到一定的低速后，停机。
  if((Duty_Target < Duty_Min) && (PeriodNow >= PeriodMax)) BLDC_STOP(); 
  PhaseOffset = Offset ;  
}
