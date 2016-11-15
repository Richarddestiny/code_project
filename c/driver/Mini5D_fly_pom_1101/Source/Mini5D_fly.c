
#define   Mini5xxDE_GLOBAL         
#include  "Mini5D_BLDC_V16.h"
#include  "Mini5D_BLDC.h"     

#define   ACMP_PH_A                CMP1_PIN_P32               
#define   ACMP_PH_B                CMP1_PIN_P34               
#define   ACMP_PH_C                CMP1_PIN_P35   
#define   DETEC_UP                 0x80                    
#define   PWM_COMPL                (0x4000ul)   
#define   PWM1_OUT_EN              0                       // PWM1 ��� A- �����ź� 
//#define   PWM1_OUT_EN              0x0200                // PWM1 ������� PWM

// P23_PWM1, P01_D6, P00_D7 ����������� A- �����ź� ////////////////////////////////////
#define   PHASE_AB        PWM1_OUT_EN | (0x0162ul)                
#define   PHASE_AC        PWM1_OUT_EN | (0x014Aul)                      
#define   PHASE_BC        PWM1_OUT_EN | (0x044Aul)                      
#define   PHASE_BA        PWM1_OUT_EN | (0x0468ul&(~0x40)) // P01_D6 ��� A- �����ź�                     
#define   PHASE_CA        PWM1_OUT_EN | (0x1068ul&(~0x40))          
#define   PHASE_CB        PWM1_OUT_EN | (0x1062ul)                     

#define   PHASE_AB_         PWM_COMPL | (0x0362ul)       
#define   PHASE_AC_         PWM_COMPL | (0x034Aul)            
#define   PHASE_BC_         PWM_COMPL | (0x0C4Aul)                          
#define   PHASE_BA_         PWM_COMPL | (0x0C68ul&(~0x40)) // P01_D6 ��� A- �����ź�                        
#define   PHASE_CA_         PWM_COMPL | (0x3068ul&(~0x40))                               
#define   PHASE_CB_         PWM_COMPL | (0x3062ul)                   
 
// ��ת����,/////////////////////////////////////////////////////////////////////////////
//������Ĭ�����ű۸ߵ�ƽ���������ű۵͵�ƽ����
uint32_t TabPhase[25] = {                        
    T0_TRG_PWM_EN | PHASE_CB  | ACMP_PH_A | DETEC_UP ,   
  
    T0_TRG_PWM_EN | PHASE_AB  | ACMP_PH_C ,                // Up-PWM  ��������
    T0_TRG_PWM_EN | PHASE_AC  | ACMP_PH_B | DETEC_UP ,   
    T0_TRG_PWM_EN | PHASE_BC  | ACMP_PH_A ,            
    T0_TRG_PWM_EN | PHASE_BA  | ACMP_PH_C | DETEC_UP , 
    T0_TRG_PWM_EN | PHASE_CA  | ACMP_PH_B ,             
    T0_TRG_PWM_EN | PHASE_CB  | ACMP_PH_A | DETEC_UP ,   
  
    T0_TRG_PWM_EN | PHASE_AB_ | ACMP_PH_C ,                // Up-PWM  ��������
    T0_TRG_PWM_EN | PHASE_AC_ | ACMP_PH_B | DETEC_UP ,   
    T0_TRG_PWM_EN | PHASE_BC_ | ACMP_PH_A ,             
    T0_TRG_PWM_EN | PHASE_BA_ | ACMP_PH_C | DETEC_UP , 
    T0_TRG_PWM_EN | PHASE_CA_ | ACMP_PH_B ,             
    T0_TRG_PWM_EN | PHASE_CB_ | ACMP_PH_A | DETEC_UP , 
                                                 // ���¶� On-PWM ���ݣ������ֻ�                                                       
    T0_TRG_PWM_EN | (0x086B)  | ACMP_PH_C ,                // A+ -> B-PWM  
    T0_TRG_PWM_EN | PHASE_AC  | ACMP_PH_B | DETEC_UP ,   
    T0_TRG_PWM_EN | (0x206E)  | ACMP_PH_A ,                // B+ -> C-PWM
    T0_TRG_PWM_EN | PHASE_BA  | ACMP_PH_C | DETEC_UP , 
    T0_TRG_PWM_EN | (0x027A)  | ACMP_PH_B ,                // C+ -> A-PWM
    T0_TRG_PWM_EN | PHASE_CB  | ACMP_PH_A | DETEC_UP ,   
                                                         
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x1162ul),              // Up-PWM 150�ȷ����ǻ�������
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0142ul),        
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x054Aul),           
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0448ul&(~0x40)),  
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x1468ul&(~0x40)),   
    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x1060ul&(~0x40)),         
//                                                         
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x3362ul),            // Up-PWM 150�ȷ�����������
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0342ul),         
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0F4Aul),        
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x0C48ul&(~0x40)), 
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x3C68ul&(~0x40)), 
//    T0_TRG_PWM_EN | PWM1_OUT_EN | (0x3060ul&(~0x40)),        
};    

// ��ת���� /////////////////////////////////////////////////////////////////////////////
//������Ĭ�����ű۸ߵ�ƽ���������ű۵͵�ƽ����

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

uint32_t  Shut_All_MOS = 0x6A | PWM1_OUT_EN ;              // д��PHCHG�ض���·MOS������ 
//=======================================================================================
void UnlockReg(void)      
{
  while((GCR->RegLockAddr & 1) == 0){                      // ��ֹ����Ͻ������ɹ�
    GCR->RegLockAddr = 0x59 ; 
    GCR->RegLockAddr = 0x16 ;    
    GCR->RegLockAddr = 0x88 ;            
  }
} 

//startup_MINI51.s�е��ã���main() ��ִ��
void SystemInit(void)      
{
    UnlockReg();                                          
    CLK->CLKDIV  = ADC_CLK_DIV_8bit(3) ;                   // ADC ʱ������Ƶ
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

void Delayus(uint32_t Time)                                // ���16777216(24λ����)
{
  uint32_t T1 = TIMER1->DR ; 
  while(((TIMER1->DR - T1) & 0xFFFFFF) < Time) ;          
}

void PWM024_0_MOS_On(void)                                 // ��PWM024���0ʱ�Ϲ���MOS��ͨ          
{
  uint32_t n ;
  PWM->PCR ^= (PCR_INV_EN(0)|PCR_INV_EN(2)|PCR_INV_EN(4)) ;          
  for(n=0; n<25; ++n){TabPhase[n] ^= 0x15; TabPhaseRev[n] ^= 0x15; } 
  Shut_All_MOS ^= 0x15 ;                                            
}

void PWM135_1_MOS_On(void)                                 // ��PWM135���1ʱ�¹���MOS��ͨ   
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
  TIMER1->CSR   =  T_CRST | T_MODE_CONTINUOUS | T_TDR_EN + 21 ;  // 22 ��Ƶ
  TIMER1->CSR  |=  T_CEN ;                                
  NVIC_SetPriority(TMR1_IRQn, 1) ;                          
  NVIC->ISER[0] = 1<<TMR1_IRQn ;                          

  ACMP->SR       = ~0 ;   //clr all flag
  ACMP->CR0      =  ACMP_CPP0_P15_0 | ACMP_EN ;                  // P1.5���Ƚ���0�������
  ACMP->CR1      =  ACMP_EN ;
  //PWM->PHCHG ���ı�Ƚ�������ѡ��
  PWM->PHCHGMASK =  PHCHG_CTL_CMP1 | P01_OUT_D6 | P00_OUT_D7 ;  //
    
  PWM->PPR = PPR_DIV45(2)   | PPR_DIV23(2)   | PPR_DIV01(2) ;   // PWM clk divide by 2, 11M
  PWM->CSR = CSR_DIV1_CH(5) | CSR_DIV1_CH(4) | CSR_DIV1_CH(3)   // CSR, no divide
           | CSR_DIV1_CH(2) | CSR_DIV1_CH(1) | CSR_DIV1_CH(0) ;                    
  PWM->PCR = PCR_GROUP_MODE | PCR_CLR_COUNTER
     | PCR_CH_EN(5) | PCR_PERIOD_MODE(5) | PCR_INV_EN(1) | PCR_CH_EN(4) | PCR_PERIOD_MODE(4) 
     | PCR_CH_EN(3) | PCR_PERIOD_MODE(3) | PCR_INV_EN(3) | PCR_CH_EN(2) | PCR_PERIOD_MODE(2) 
     | PCR_CH_EN(1) | PCR_PERIOD_MODE(1) | PCR_INV_EN(5) | PCR_CH_EN(0) | PCR_PERIOD_MODE(0);
  PWM->CMR0  = 0 ;       //��ʼռ�ձ�Ϊ0                                 
  PWM->CMR1  = 0 ;         //��ʼռ�ձ�Ϊ0                             
//  PWM->PDZIR = 0x111111 ;                 // ����ʱ�� 18�� PWM CLK,Լ1.6us

  PWM024_0_MOS_On() ;                 // PWM024 �����ʱ MOS ��ͨ,���ô˺������������     
  PWM135_1_MOS_On() ;                 // PWM135 �����ʱ MOS ��ͨ,���ô˺������������ 
   
  if(Shut_All_MOS & 0x20) GPIO0->DOUT = 0xFD ;             // D5_P0.4 �ض� MOS
  else GPIO0->DOUT = 0xED ;                                
  if(Shut_All_MOS & 0x40) GPIO0->DOUT |= 0x02 ;            // D6_P0.1 �ض� MOS
  
  GPIO2->DOUT     = Shut_All_MOS << 2 ;                    
  
  PWM->PHCHGNXT   = Shut_All_MOS ;                         
  PWM->PHCHG      = Shut_All_MOS ;                  
  PWM->TRGCON0    = 0x0008 ;    //Ԥ��    Enable PWM Trigger ADC Function While Channel0��s Counter Matching 0                        
  PWM->PIIR       = ~0 ;  //clear all PWM  Interrupt Indication flag
//  PWM->PFBCON   = (Shut_All_MOS<<24) | BRK1_SOURCE_CPO0 | BRK1_EN; //BRK  cfg   
  PWM->PIER       = PIER_BRKE_IE + PIER_PIE(0);  //break int and ch0 int enable                                     
  PWM->INTACCUCTL = 0 ;   //�ۼ��жϼĴ�������0���ۼ�                            
  PWM->POE        = 0x3F ;  //enable 6 channel output                             
  
  NVIC_SetPriority(FB_IRQn, 0) ;                        
  NVIC->ISER[0]  = 1 << FB_IRQn ;                  
  
  pPhaseEnd = &TabPhase[6] ;                               // �Ȱ���ת��ʼ����Щֵ  
  pPhase    = &TabPhase[1] ;                   
}

void BLDC_STOP(void)                                       // ���������
{                                             
  TIMER1->CSR  &= ~T_IE ;                                  // ���ж�
  PWM->PHCHGNXT =  Shut_All_MOS ;                        
  PWM->PHCHG    =  Shut_All_MOS ;   
  PeriodNow     =  11000000 ;                              // ���ֵ���������ת��Ϊ0  
  Delayus(20) ;                                           
}

//=======================================================================================
void Motor_Beep(uint32_t Duration, uint32_t Period) //����ʱ���Ͱ�����(�൱������),΢����
{
  uint32_t temp32 = TIMER1->DR ;  
    
  if(pPhase > (pPhaseEnd-3)) pPhase = pPhaseEnd-5 ;  //������漸������߽硣       
  PWM->CMR0 = Duty_Current ;                               // Duty_Current ��ֵ�������
  do{
    PWM->PHCHG = *(pPhase+3) & (~T0_TRG_PWM_EN) ;    //DISABLE changed by T0
    Delayus(Period) ;  BLDC_STOP() ;
    PWM->PHCHG = *pPhase & (~T0_TRG_PWM_EN) ;          
    Delayus(Period) ; BLDC_STOP() ; 
  }while(((TIMER1->DR - temp32) & 0xFFFFFF) < (Duration<<1)) ; 
  Delayus(30000) ; 
}

//// ��ת, ͣת, �������ѹ /////////////////////////////////////////////////////////////
void  BLDC_Control(void)          
{ 
  int32_t DutyInc = 0 ;                       
  static uint32_t  OutImg = 0, DutyTim = 6000;                            
  
  if((TIMER1->CSR & T_IE) == 0)
  {  // ûת����תʱ�����ж�     
    if(Duty_Target >= (Duty_Min + 2*PWM_one_percent)) //Duty_Target��Duty_Min ��
     {   
           //  StartOffset  = 12 ;                                        // ��תʱ�����ֻ� PWM       
          PWM->CMR1    = Duty_Min ;    
          PWM->CMR0    = Duty_Min ;   Delayus(100);                  // ���һ�� PWM ���ں����                               
          pPhaseEnd    = &TabPhase[6] ;                              // ʹ������TabPhase[1] ~ TabPhase[6]                   
          pPhase       = &TabPhase[1] ;                              // ��һ����ת
          ACMP->CR1    =  ACMP_EN ;                          //����ʱû��20mv�Ļز�ACMP_HYST_EN
          //����Сռ�ձ�����
          Duty_Current = Duty_Min  ;                                 // ��ǰռ�ձ�,���԰���Դ��ѹ����   

          // PeriodShrink = 32 ;                                      // ��ת�׶�, ����ʱ�䲻��
          // PeriodMax    = 30*1000 ;
          Wait_Zero(TabPhase[6], 10*1000) ;     //
          BLDC_Start(PeriodMax, 600) ;                               // ��ת,����Ϊ��ת���� 
          // GPIO54 ^= 1;  ����ָʾ    
    } 
  }  
  else
  {   // ��ת  
    if(OutImg != PWM->PHCHG)
    { 
      OutImg = PWM->PHCHG ;              // �ջ���
      if(Duty_Current < Duty_Target)      ++Duty_Current ;              // �ӿ���ѹ�ٶ�

      //�������ڣ����ռ�ձȲ��ܳ���35%
      if(StepCount < 12){                                       
        if(Duty_Current > 35*PWM_one_percent) Duty_Current -= 5*PWM_one_percent ;
                                         
      }
      else{                                                            
        if(StepCount > 300) ACMP->CR1 = ACMP_EN | ACMP_HYST_EN;  // ������
        
        // ���ٴ���Ŵ�, �û������� PeriodNow ���㵱ǰ�ٶ�,���ռ�ձ�����DutyInc                                
      } 
      
      if(OutImg & PWM_COMPL) DutyTim = 3000 ;       //�����ź�ʱ            
      else DutyTim = 6000 ;         
    }

    //����ʱ��ÿ�����Ӽ���ֵ1������ʱֱ��һ����λ��
    if(Duty_Target <= Duty_Current)     Duty_Current = Duty_Target;  // �����»�����һ������    
    else DutyInc = 1;    
      
    if(Duty_Current > 70*PWM_one_percent) BLDC_Add_Duty(DutyInc, 600, 0) ;      
    else if(Duty_Current > 35*PWM_one_percent) BLDC_Add_Duty(DutyInc, 1200, 0) ;        
    else BLDC_Add_Duty(DutyInc, DutyTim, 0) ;       

   //����ռ�ձ�
    PWM->CMR0 = Duty_Current ;                                                            
    PWM->CMR1 = Duty_Current ;  
        
    if((TIMER1->CSR & T_IE) == 0)       Delayus(100000);               // �ո�ͣת,��100ms          
  }
}

//// �ѷ���FiFo���� /////////////////////////////////////////////////////////////////////
// �β� :  *pStr, �ַ�ָ��
//          Cnt , �ַ�����
// ���� :   pStr, ʣ������ַ���ַ
//          0   ,  �ַ���������
uint8_t* Tx0FillFiFo(uint8_t*pStr, uint32_t Cnt)
{
  if(Cnt){                                                   
    do{
      if( UART0->FSR & TX_FIFO_FULL ) break ;              //FiFo���˾��˳�ѭ��        
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
    *pStr-- = Asc[temp32u - Number*10];                    // �ȵõ���λ 
    ++Cnt ;
  }while(Number); 
  return Cnt ;
}

/////////////////////////////////////////////////////////////////////////////////////////
//����һDutyĿ��ֵ, ��������ʱ���ټ�   
__weak void BLDC_Add_Duty(int32_t DutyInc, uint32_t Delay, uint32_t Offset) 
{
  static uint32_t  Tmr1_Tick = 0 ;                                             
    
  if(((TIMER1->DR - Tmr1_Tick) & 0xFFFFFF) >= Delay){                        
    Duty_Current += DutyInc ;                                                       
    Tmr1_Tick     = TIMER1->DR ;                            
  } 

  // �����Źص��ˣ����ٶȵ������󣬲�ִ��ͣ����
  //���Źص���ռ�ձ�ֱ�ӵ�0���ȵ����Ȼ���ٵ�һ���ĵ��ٺ�ͣ����
  if((Duty_Target < Duty_Min) && (PeriodNow >= PeriodMax)) BLDC_STOP(); 
  PhaseOffset = Offset ;  
}
