//// ====================================================================================
#include  "Mini5xxDE.h" 
uint32_t  DetectUpBit = 0x80 ;              
uint32_t  AcmpOutBit  = ACMP_OUT1 ;        

uint32_t *pPhaseEnd ;  
uint32_t  PeriodMax  = 60*1000 ;      //60ms
uint32_t  PhaseAngle = 24 ;    //��ʱ�����
uint32_t  PeriodShrink ;
uint32_t  PhaseOffset ; //��ȡֵ0 6 12��ѡ��ͬ������ʽ��
uint32_t  volatile ZeroDeadline ; //������ĳ�ʱʱ��

uint32_t *pPhase    ;       
uint32_t  PeriodNow ;  //60������ڵĳ��ȣ����λ�����                 
uint32_t  StepCount ;  //������ת�˶��ٲ��������ϲ��������������תһ����
uint32_t  ZeroTick ;  //���㻻�����ڣ����Լ�¼��㡣

uint32_t  StartOffset = 0;  //û���ô�
// BLDC ��ת, ��������ת���� ////////////////////////////////////////////////////////////  
void BLDC_Start(uint32_t FirstPeriod, uint32_t DelayMin)      
{ 
  PeriodNow     =  FirstPeriod ;                       
  TIMER0->CSR  |=  T_CRST ;                               
  TIMER0->CMPR  =  PeriodNow ;          
  TIMER0->CSR  |=  T_CEN ;                                 // ��������
  ZeroTick      =  TIMER1->DR ;                   
  TIMER1->CMPR  =  ZeroTick + DelayMin ;           // FirstDelay����������ʱ         
  TIMER1->CSR  |=  T_IE ;                                  // ʹ���ж� 
  StepCount     =  0 ;                                        //���������������ںͺ���                            
  PWM->PHCHGNXT = *pPhase ;                                
  PWM->PHCHG    = *pPhase ;                                // MOS��ʼ��ͨ   
  PhaseOffset   =  StartOffset ;    
} 
///////////////////////////////////////////////////////////////////////////////////////// 
// ����ִ���ж�ʱ TIMER0->CMPR����0xFFFFFF,дDeadLine_Detec = TIMER0->DR����ʱ��ֹ���0
// �жϷ��غ�,������ StepCount, PeriodLast, PeriodNow, PHCHGNXT, ����û����
// Timer0 ������ CMPRʱ�Ż���, �´μ��0�� Timer1������ CMPRʱ
void TMR1_IRQHandler(void)                                    
{  
  uint32_t  PeriodLast ;

  TIMER0->CMPR = ~0 ;          //     ~0 is 0xFFFFFFFF                     
  TIMER1->ISR  = ~0 ;                                           // Clear all interrupt flag

  //ͣ���ڴ�ֱ��ץ��������߳�ʱ
  //��ʱʱ��Ϊ�������ڵ�����
  ZeroDeadline =  (PeriodNow << 1);
  do{
        if((*pPhase) & DetectUpBit){                                                
          if(ACMP->SR & AcmpOutBit){ ++StepCount; break; }           // up zero      
        }
        else{                                                                    
          if((ACMP->SR & AcmpOutBit)==0){ ++StepCount; break; }      // down zero                      
        }
  }while(TIMER0->DR < ZeroDeadline) ;

  //�����60�㻻�����ڳ���
  PeriodLast = (TIMER1->DR - ZeroTick) & 0xFFFFFF ;
  ZeroTick   = TIMER1->DR ;
  PeriodNow  = (PeriodNow + PeriodLast)>>1 ;
  
  if(PeriodNow > PeriodMax) PeriodNow = PeriodMax ;  

  //���û�����ʱ�ǣ�PeriodNow * 24 /64, Ϊ����������㣬60����64
  TIMER0->CMPR =  ((PeriodNow*PhaseAngle) >> 6) ;    // Set TIMER0 period
  //�����´εĻ����⿪���㣬3/4�����ڡ�
  TIMER1->CMPR = TIMER1->DR + ((PeriodNow*3)>>2) ;               // Set TIMER1 interrupt
  
  if(++pPhase  > (pPhaseEnd))   pPhase -= 6 ;    
  PWM->PHCHGNXT = *(pPhase + PhaseOffset);                       // phase transfer
}

uint32_t Wait_Zero(uint32_t Phase, uint32_t TimOV) 
{
  return 0 ;
}   
