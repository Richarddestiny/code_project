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

#define  ADC0_SPEED                0x01          // 调速旋钮      
#define  ADC1_CURRENT              0x02          // 平均电流   
#define  ADC2_VIN                  0x04          // 输入电压 
#define  ADC3_TEMP                 0x08          // 温度
 
EXT_BLDC volatile uint32_t gFlag ;   
#define  Flag_VOLTAGE_LOW          0x0001        // 电压太低   
#define  Flag_TEMPERATURE_OVER     0x0002        // 超温 

#define  Flag_PCB_TEMP_OV          0x0010        // PCB 超温
#define  Flag_BATT_LIMIT           0x0020        // 电池不对
#define  Flag_LOW_PWR              0x0040        // 掉电

#define  Flag_STAGE_S0             0x0100        // 启转标志 
#define  Flag_PPM_OK               0x0200          
  
EXT_BLDC  uint32_t ACMP_Flag ;  
#define  ACMP_EVER_SHORT           0x0001        // ACMP 检到过短路 

EXT_BLDC uint32_t          PWM_one_percent ;     // PWM 1% 时对应的值
EXT_BLDC uint32_t          Duty_Target ;         // 占空比目标值,控制转速
EXT_BLDC uint32_t          Duty_Min ;            // 占空比最小值,再小就停转
EXT_BLDC uint32_t          Duty_Current ;        // 占空比当前值

EXT_BLDC uint32_t          Adc_Current ;         // 实测电流值
EXT_BLDC uint32_t          Current_Limit ;       // 限流值
EXT_BLDC uint32_t          Current_Zero  ;       // 0电流时的 ADC 值
EXT_BLDC uint32_t          Tick_Start ;          // 启转时刻

EXT_BLDC uint32_t volatile TickRelay ;           // 每次 SysTick 中断减1 

void  PWM_ACMP0_T0_T1_Init(void) ;               // T0,T1,PWM 时钟都选了 HCLK = 22M  
void  BLDC_Control(void) ; 
void  BLDC_STOP(void) ;                          // 停止监控 BLDC, 停机
void  BLDC_Brake(uint32_t BreakDuty) ;           // 电刹车, 参数为刹车占空比
void  Motor_Beep(uint32_t, uint32_t) ;           // 电机蜂鸣

void      Delayus(uint32_t Time) ;               // 24位, 最长约16.5秒 (1MHz计数) 
uint8_t*  Tx0FillFiFo(uint8_t*, uint32_t) ;      // 填满 Tx0 FiFo
uint32_t  HexToStr(uint32_t, uint8_t*) ;         // 数字转换成字符
#endif   // __Mini5xxDE_BLDC_H 
