//// ====================================================================================
#include  "Mini5xxDE.h" 
uint32_t  DetectUpBit = 0x80 ;              
uint32_t  AcmpOutBit  = ACMP_OUT1 ;        

uint32_t *pPhaseEnd ;  
uint32_t  PeriodMax  = 60*1000 ;      //60ms
uint32_t  PhaseAngle = 24 ;    //延时换向角
uint32_t  PeriodShrink ;
uint32_t  PhaseOffset ; //可取值0 6 12，选择不同驱动方式。
uint32_t  volatile ZeroDeadline ; //检测过零的超时时间

uint32_t *pPhase    ;       
uint32_t  PeriodNow ;  //60°电周期的长度，两次换向间隔                 
uint32_t  StepCount ;  //启动后转了多少步。基本上不会溢出。能连续转一个月
uint32_t  ZeroTick ;  //计算换向周期，用以记录起点。

uint32_t  StartOffset = 0;  //没有用处
// BLDC 启转, 参数是启转周期 ////////////////////////////////////////////////////////////  
void BLDC_Start(uint32_t FirstPeriod, uint32_t DelayMin)      
{ 
  PeriodNow     =  FirstPeriod ;                       
  TIMER0->CSR  |=  T_CRST ;                               
  TIMER0->CMPR  =  PeriodNow ;          
  TIMER0->CSR  |=  T_CEN ;                                 // 启动计数
  ZeroTick      =  TIMER1->DR ;                   
  TIMER1->CMPR  =  ZeroTick + DelayMin ;           // FirstDelay，检测过零延时         
  TIMER1->CSR  |=  T_IE ;                                  // 使能中断 
  StepCount     =  0 ;                                        //用来区分启动早期和后期                            
  PWM->PHCHGNXT = *pPhase ;                                
  PWM->PHCHG    = *pPhase ;                                // MOS开始导通   
  PhaseOffset   =  StartOffset ;    
} 
///////////////////////////////////////////////////////////////////////////////////////// 
// 正在执行中断时 TIMER0->CMPR等于0xFFFFFF,写DeadLine_Detec = TIMER0->DR会因超时中止检过0
// 中断返回后,更新了 StepCount, PeriodLast, PeriodNow, PHCHGNXT, 但还没换相
// Timer0 计数到 CMPR时才换相, 下次检过0到 Timer1计数到 CMPR时
void TMR1_IRQHandler(void)                                    
{  
  uint32_t  PeriodLast ;

  TIMER0->CMPR = ~0 ;          //     ~0 is 0xFFFFFFFF                     
  TIMER1->ISR  = ~0 ;                                           // Clear all interrupt flag

  //停留在此直到抓到过零或者超时
  //超时时间为换相周期的两倍
  ZeroDeadline =  (PeriodNow << 1);
  do{
        if((*pPhase) & DetectUpBit){                                                
          if(ACMP->SR & AcmpOutBit){ ++StepCount; break; }           // up zero      
        }
        else{                                                                    
          if((ACMP->SR & AcmpOutBit)==0){ ++StepCount; break; }      // down zero                      
        }
  }while(TIMER0->DR < ZeroDeadline) ;

  //计算出60°换向周期长度
  PeriodLast = (TIMER1->DR - ZeroTick) & 0xFFFFFF ;
  ZeroTick   = TIMER1->DR ;
  PeriodNow  = (PeriodNow + PeriodLast)>>1 ;
  
  if(PeriodNow > PeriodMax) PeriodNow = PeriodMax ;  

  //设置换向延时角，PeriodNow * 24 /64, 为避免除法运算，60改用64
  TIMER0->CMPR =  ((PeriodNow*PhaseAngle) >> 6) ;    // Set TIMER0 period
  //设置下次的换向检测开启点，3/4个周期。
  TIMER1->CMPR = TIMER1->DR + ((PeriodNow*3)>>2) ;               // Set TIMER1 interrupt
  
  if(++pPhase  > (pPhaseEnd))   pPhase -= 6 ;    
  PWM->PHCHGNXT = *(pPhase + PhaseOffset);                       // phase transfer
}

uint32_t Wait_Zero(uint32_t Phase, uint32_t TimOV) 
{
  return 0 ;
}   
