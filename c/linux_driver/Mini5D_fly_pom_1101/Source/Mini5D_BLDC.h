//=======================================================================================
// Copyright Nuvoton(SH) Technology Corp. All rights reserved. 2015/01/01   
//=======================================================================================
#ifndef  __Mini5xxDE_BLDC        
#define  __Mini5xxDE_BLDC             
#include  "Mini5xxDE.h"    

#ifndef  Mini5xxDE_GLOBAL            
 #define  EXT_BLDC  extern                       
#else  
 #define EXT_BLDC       
#endif

#define  ADC0_SPEED                0x01          // ������ť      
#define  ADC1_CURRENT              0x02          // ƽ������   
#define  ADC2_VIN                  0x04          // �����ѹ 
#define  ADC3_TEMP                 0x08          // �¶�
 
EXT_BLDC volatile uint32_t gFlag ;   
#define  Flag_VOLTAGE_LOW          0x0001        // ��ѹ̫��   
#define  Flag_TEMPERATURE_OVER     0x0002        // ���� 

#define  Flag_PCB_TEMP_OV          0x0010        // PCB ����
#define  Flag_BATT_LIMIT           0x0020        // ��ز���
#define  Flag_LOW_PWR              0x0040        // ����

#define  Flag_STAGE_S0             0x0100        // ��ת��־ 
#define  Flag_PPM_OK               0x0200          
  
EXT_BLDC  uint32_t ACMP_Flag ;  
#define  ACMP_EVER_SHORT           0x0001        // ACMP �쵽����· 

EXT_BLDC uint32_t          PWM_one_percent ;     // PWM 1% ʱ��Ӧ��ֵ
EXT_BLDC uint32_t          Duty_Target ;         // ռ�ձ�Ŀ��ֵ,����ת��
EXT_BLDC uint32_t          Duty_Min ;            // ռ�ձ���Сֵ,��С��ͣת
EXT_BLDC uint32_t          Duty_Current ;        // ռ�ձȵ�ǰֵ

EXT_BLDC uint32_t          Adc_Current ;         // ʵ�����ֵ
EXT_BLDC uint32_t          Current_Limit ;       // ����ֵ
EXT_BLDC uint32_t          Current_Zero  ;       // 0����ʱ�� ADC ֵ
EXT_BLDC uint32_t          Tick_Start ;          // ��תʱ��

EXT_BLDC uint32_t volatile TickRelay ;           // ÿ�� SysTick �жϼ�1 

void  PWM_ACMP0_T0_T1_Init(void) ;               // T0,T1,PWM ʱ�Ӷ�ѡ�� HCLK = 22M  
void  BLDC_Control(void) ; 
void  BLDC_STOP(void) ;                          // ֹͣ��� BLDC, ͣ��
void  BLDC_Brake(uint32_t BreakDuty) ;           // ��ɲ��, ����Ϊɲ��ռ�ձ�
void  Motor_Beep(uint32_t, uint32_t) ;           // �������

void      Delayus(uint32_t Time) ;               // 24λ, �Լ16.5�� (1MHz����) 
uint8_t*  Tx0FillFiFo(uint8_t*, uint32_t) ;      // ���� Tx0 FiFo
uint32_t  HexToStr(uint32_t, uint8_t*) ;         // ����ת�����ַ�
#endif   // __Mini5xxDE_BLDC_H 
