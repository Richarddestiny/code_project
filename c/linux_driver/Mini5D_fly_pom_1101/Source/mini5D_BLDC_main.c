#include  "Mini5D_BLDC_V16.h"
#include  "Mini5D_BLDC.h"               

uint32_t volatile Duty_Command ;                           // Duty����, ��Χ 0 ~ CNR
uint32_t  volatile Pulse_Current ;                         // PPM ��ǰֵ
uint32_t  volatile PulseTick ;                             // PPM ����ʱ�� 
uint32_t           Pulse_Min    = 1000 ;                   // PPM ��Сֵ,ȱʡ1000us
uint32_t           Pulse_Length = 1000 ;                   // PPM ���ֵ����Сֵ�Ĳ�
////=====================================================================================             
////UART0 Initial ///////////////////////////////////////////////////////////////////////
void UART0_Init(uint32_t Baud)                  
{
  UART0->FUN_SEL = 0  ;                                    //UART ģʽ
  UART0->LCR     = 0x0007 ;                                //8bit, 2-stop, no parity
  UART0->BAUD    = 0x30000000 + 22118400/Baud - 2 ;        //UART ʱ�� 22M
  UART0->FCR     = 0x00020020 ;                            //RTS trigger:8, FIFO trigger:8 BYTE
  UART0->IER     = 0 ;                                   
}
/////////////////////////////////////////////////////////////////////////////////////////
// Configure P2.23456 & P0.4 as PWM function pin.  configure P1.0345 as ACMP input pin
void GPIO_Init(void) 
{                                          // P01���ó�OUTģʽ,������PHCHG����,QBģʽ����      
  // P0.014567 ����, GPIO0->DOUT �ĳ�ֵ�ں���PWM_ACMP0_T0_T1_Init()��д��
  GPIO0->PMD  = Px7_QB | Px6_QB | Px5_QB | Px1_OUT | Px0_OUT ;         // P0.1 ����Ϊ A-
  GCR->P0_MFP = 0xF010 ;          // P0.4_PWM5, P0.0567_SPI, ��P04����
                                
  //GPIO1->DOUT = 0 ;//
  GPIO1->PMD  = Px5_OUT | Px4_OUT |Px3_OUT | Px2_OUT | Px0_OUT ;     // P1.02345 ���
  GPIO1->OFFD = 0x00FF0000 ;                               //P1 ��������ͨ��ȫ�ر�
  GCR->P1_MFP = 0x303D ;                                   // P1.023_ADC123, P1.45_ACMP0
                           // GPIO2->DOUT �ĳ�ֵ�ں���PWM_ACMP0_T0_T1_Init()��д
  GCR->P2_MFP = 0x7C00 ;                                   // P2.23456,���,PWM01234
  
//��������Ϊ����ʱ��ʲô�����ӣ����ŵ�ѹ������Ư��������©�磬Ҳ���Ӹ���
//����Ϊ׼˫�����1���ͱ����˵�ѹ����Ư��PCB�Ϳ���ʲô�������ˡ�
  GPIO3->DOUT = 0xFF ;                                     // P3.012456 ����
  GPIO3->OFFD = 0x00BF0000 ;                               // ֻ��ͨ36����������ͨ��
  GPIO3->PMD  = Px6_QB  ;                            // P3.0145_CMP1,P3.6_GPIO, 
  GCR->P3_MFP = 0x01003376 ;                                  

  GPIO4->DOUT = 0xFF ;
  GPIO4->PMD  = Px7_QB | Px6_QB ;                          // P4.67,ICE�ӿ���¼
  GCR->P4_MFP = 0 ;                                

  GPIO5->DOUT = 0xFF ;                                     // P5.012345 ����,(QFN33û��P5.5)
  GPIO5->PMD  = Px5_QB | Px4_QB | Px2_QB | Px1_QB | Px0_QB ;  //��δʹ��  
  GCR->P5_MFP = 0x0008 ;                                   // P5.3_ADC0

   //P36��PPM�źš�
  GPIO3->ISR  = 0xFF ;                                     // Clear ISR  
  GPIO3->IMD  = 0 ;                                        // Edge interrupt mode
  GPIO3->IER  = 0x00400040 ;                               // λ16~23����ʹ��,0~7�����ж�ʹ��
  GPIO3->DBEN = 0x40 ;                                     // 1 mean De-bounce
  GPIODBNCE   = 0x2B ;                                             // De-bounce time = 2048HCLK

  NVIC_SetPriority(GPIO234_IRQn, 2);                       // Interrupt priority = 2 
  NVIC->ISER[0] = 1<<GPIO234_IRQn ;                                

  //���û��ʹ��
  GPIO5->ISR  = 0xFF ;                                     // Clear ISR  
  GPIO5->IMD  = 0 ;                                        // Edge interrupt mode
  GPIO5->IER  = 0x00000004 ;                               // λ16~23����ʹ��,0~7�����ж�ʹ��
  GPIO5->DBEN = 0x04 ;                                     // P5.2 Enable De-bounce

  NVIC_SetPriority(EINT1_IRQn, 2) ;                        //Interrupt priority = 2 
//  NVIC->ISER[0] = 1<<EINT1_IRQn ;                        // ����
}
//=======================================================================================
// MAIN function                                                               
//---------------------------------------------------------------------------------------
int main(void) 
{  
  uint32_t  Adc_Vin, Adc_Tempera, temp32, LastDuty = ~0 ; 
    
  PWM_ACMP0_T0_T1_Init() ;      
  ADC->CR      = ADCR_EN ;                                 // ����ʹ��ADC 
  ADC->ADSAMP  = 4 ;                                       // ����ʱ�� 9 CLK
  ADC->ADTDCR  = 0 ;                                 //ADC Trigger Delay Controller Register 
                 //PWMƵ��: 5_22.1KHz, 6_18.4KHz, 7_15.8KHz, 8_13.8KHz, 10_11KHz, 14_7.9K
  PWM_one_percent = 8 ;                                                     
  PWM->CNR0       = 100*PWM_one_percent - 1 ;              // ���� PWM ����                  
  PWM->CNR1       = 100*PWM_one_percent - 1 ;                     
  Duty_Min        = 8*PWM_one_percent ;
    
  UART0_Init(115200) ; 
  GPIO_Init() ;          
  // ��� BOD ��ѹ�Ƿ�����Ϊ 2.7V ���±��ָ�λ //////////////////////////////////////////
  CLK->AHBCLK |= CLK_ISP_EN ; 
  FMC->ISPCON |= FMC_CFGUEN | FMC_EN_ ; 
  FMC->ISPADR  = 0x00300000 ;                              // Config0
  FMC->ISPCMD  = FMC_READ_0 ;                              // Read config0 
  FMC->ISPTRG  = 1 ;
  while(FMC->ISPTRG & 1) ; 
  temp32 = FMC->ISPDAT ;//��ȡ����ֵ
  
  //����BOD��ѹû������Ϊ2.7V��������д���µ�2.7V���á�
  if((temp32 & 0x00F004C1) != 0x00200481){           
    FMC->ISPCMD  = FMC_ERASE ;   
    FMC->ISPTRG  = 1 ;
    while(FMC->ISPTRG & 1) ;
    FMC->ISPDAT  = (temp32 & 0xFF0FFF3F) | 0x00200481 ;
    FMC->ISPCMD  = FMC_WRITE ;                             // �� BOD ��ѹд�� 2.7V ���¸�λ
    FMC->ISPTRG  = 1 ;
    while(FMC->ISPTRG & 1) ;
    GCR->IPRST_CTL1 = 1 ;                                  // Chip Reset 
  }

  //�رռĴ�����д
  FMC->ISPCON &= ~(FMC_CFGUEN | FMC_EN_) ; 
  CLK->AHBCLK &= ~CLK_ISP_EN ;  
  LOCKREG();  
  ///////////////////////////////////////////////////////////////////////////////////////
  gFlag     = 0 ;                                          // ��0��־
  ACMP_Flag = 0 ;                                          
  TIMER1->EXCON = TEX_DEBO_EN |TEX_EN | TEX_CAP_FALL_RISE; // ��ʼ����PPM ����
  Duty_Current  = 15*PWM_one_percent ;                      
  Motor_Beep(50000,900) ;  ++pPhase ;                      // AB ���� 50ms 
  Motor_Beep(50000,800) ;                                  // AC ����

  //�����ŵ�ƥ��
  if(Pulse_Current > 1600){
    Pulse_Length = Pulse_Current ; 
    while(Pulse_Current > 1400) ; 
    Delayus(500000) ;                                     
    Motor_Beep(50000,900) ;
    Pulse_Min    = Pulse_Current ;  
    Pulse_Length = Pulse_Length - Pulse_Min - 50 ; 
    Motor_Beep(50000,800) ;                               
  }
  Pulse_Length = ((100*PWM_one_percent)<<16)/Pulse_Length ;//����16Ϊ�˾��ȡ�
  Motor_Beep(50000,600) ;                                 

  //���˳�ʼ����ɣ����濪ʼ�������    
  ///////////////////////////////////////////////////////////////////////////////////////
  BLDC_STOP() ;                                                                                       
  ADC->CR |= ADCR_START_BUSY ;                             // ���� ADC
  while(1){ 
    //// ����ť,�¶�,��ѹ,���� ========================================================== 
    temp32 = ADC->DR ;    
    if(temp32 & 0x20000){    // ADC������Ч,����0,�����ADC�Ĵ�����ʱ,���ܻᱻ��������0  
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
      //�������ADC ת����Ҳ������PWM����
      ADC->CR |= ADCR_START_BUSY ;     
    }
    ////=================================================================================
    if(gFlag & Flag_PPM_OK){                               // ��PPM�ź�, ����ռ�ձ�
      if(Pulse_Current <= Pulse_Min) Duty_Command = 0 ; 
      //Pulse_Length refer to Line 128
      else Duty_Command=((Pulse_Current - Pulse_Min)*Pulse_Length)>>16; // �䵽 0 ~ CNR0����   
      Duty_Target = Duty_Command ;   
    }
    
    temp32 = PulseTick ; 
    if(((TIMER1->DR - temp32)&0xFFFFFF) > 3000*1000){      // 3 ���� PPM �����ź�
      PulseTick   = TIMER1->DR ; 
      Duty_Target = (Duty_Target*7) >> 3 ;                 // Ŀ��ռ�ձȽ�Ϊ 87.5%
      gFlag      &= ~Flag_PPM_OK ;   
    }
    
    Duty_Target = 15*PWM_one_percent ;                     // ���ڲ���
    
    if((LastDuty ==0)&&(Duty_Target)){                     //��ͣתʱ ͻȻ���¼ӵ�
      if(PeriodNow > 1500) BLDC_Start(PeriodMax, 0);       // ���ٵ����ٶ�
      if(Duty_Current < Duty_Min) Duty_Current = Duty_Min ;    //ռ�ձ�������Duty_Min                              
    }
    LastDuty = Duty_Target ;   
    
    BLDC_Control() ;                           // BLDC ��ֹ����������  
    if(PeriodNow < (10000000ul/250000ul)) BLDC_STOP();     // �ѳ���250Krpm ? ͣת        
    
    ////===== ��������Ŵ� ==============================================================
  } 
}

void GPIO234_IRQHandler(void)                              // �����������  
{                                                          
  uint32_t temp32 ;                                    
  //When TEXIF flag is set to 1, the current TDR value will be auto-loaded into this 
  // TCAP filed immediately.
  // ����config ��Line 109
  // Pulse_Current range from 1000 t0 2000, PPM spec.
  
//  if(GPIO3->ISR & 0x40){                                 // GPIO234�������ж�,�˾���
    if(GPIO3->PIN & 0x40) PulseTick = TIMER1->CAP ;        // PPM ����ʱ��
    else{//�½���ʱ��
      temp32 = ((TIMER1->CAP - PulseTick)&0xFFFFFF) ;      // PPM ���,΢����
      if(temp32 < 2500){Pulse_Current = temp32; gFlag |= Flag_PPM_OK; }          
    }
    TIMER1->EXISR = ~0 ;                                   // ���жϱ�־
//  }       //  End of "if(GPIO3->ISR & 0x40)"
  GPIO3->ISR = ~0 ;                                        // ���жϱ�־
}

//����������ӡ��Ϣ
void SysTick_Handler(void)                                 // 20ms �ж�һ��
{   
  ++TickRelay ; 
#if 1                                                      // ��ӡ�ٶ���Ϣ
  {
    uint32_t static Cnt = 0, flag = 0 ;
    uint8_t  str[] = "Duty=   %         rpm\n\r" ; 
      
    if(++Cnt >= 25){  Cnt = 0 ; flag ^= 1 ;                // 20ms X 25 =0.5S
      if(flag == 0){  
        HexToStr(10000000/PeriodNow, &str[16]) ;           // ת��ת�����ַ�
        Tx0FillFiFo(&str[10],13) ;                         // д�� Tx0
      }
      else{  
        HexToStr(PWM->CMR0/PWM_one_percent, &str[7]) ;     // ռ�ձ�ת�����ַ�
        Tx0FillFiFo(str,10) ;                              // д�� Tx0
      }
    }      
  }
#endif  
}
