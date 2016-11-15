//=======================================================================================
// Copyright Nuvoton(SH) Technology Corp. All rights reserved. 2016/06/06   
//=======================================================================================
// Timer01时钟源必须选HCLK或与其选同一时钟源, Timer01寄存器分频系数必须相同且大于10
// PWM 时钟必须是 HCLK 的至少二分频
#ifndef  __Mini5xxDE_BLDC_V16         
#define  __Mini5xxDE_BLDC_V16                      
#include <stdint.h>       

//// 起转 ///////////////////////////////////////////////////////////////////////////////
// 输入: FirstPeriod, 启转周期,即第一次换相时间  
//       DelayMin,    启转阶段退磁时间, 换相到检过0之间的延时 
// 说明: 调用前占空比CMR0,当前指针pPhase,末相指针pPhaseEnd等应先赋值,有以下几种调用方式:
// 1>, 静止启转：BLDC_Start(PeriodMax,非0),PeriodMax周期最大值变量
// 2>, 低速启转: BLDC_Start(Period,   非0),Period是测出的周期,参数2建议不大于参数1的一半 
// 3>, 高速启转: BLDC_Start(Period,     0),测出周期 Period 值不能大于 PeriodMax 
// 4>, 加速调整: BLDC_Start(PeriodMax,  0),PeriodMax周期最大值变量,
// 一,启转前芯片不控制 BLDC ((TIMER1->CSR & T_IE)==0)且 BLDC 完全不转,请用方式1>启转
// 二,启转前芯片不控制 BLDC ((TIMER1->CSR & T_IE)==0)但 BLDC 已在转动,两次调用Wait_Zero()
//    测出换相周期做参数1,再让pPhase指向下一相后立即按方式2>或3>调用函数,省去了托动过程,
//    BLDC直接同步转动,方式2>比方式3>多做滤波调整以保证低速下能可靠同步
// 三,低速下电压由较小值增加较快时,方式4>的调用可加快提速,停转状态这种调用无效,不会启转
// 启转前可以花时间检测一下是否在转,若确定没转,按方式1>启转,启转延时最短
void BLDC_Start(uint32_t FirstPeriod, uint32_t DelayMin);     

//// 增加PWM占空比 //////////////////////////////////////////////////////////////////////
// 输入: DutyInc, PWM占空比增量,即占空比增加这个值, 可正可负, 注意CNR是16位的
//       Delay,  上次此函数改变Duty_Current在Delay之前,这次调用才会再次改变Duty_Current
//       PhOffset, PhaseOffset的新值(启转ActiveNum步内无效),下次检过0用新值
// 说明：此函数包括停转判断,最好每次换相都能执行到 
void BLDC_Add_Duty(int32_t DutyInc, uint32_t Delay, uint32_t PhOffset) ; 

//// 停转 BLDC,等待指定相过0点,返回时刚好是过0时刻点 /////////////////////////////////// 
// 输入：Phase, 等此相过0点,关 MOS 10us后开始检测,可在调用函数前停转增加延时 
//       TimOV, 超时时间,用 Timer1 定时
// 返回: 测到过0点返回非0,否则返回0
uint32_t Wait_Zero(uint32_t Phase, uint32_t TimOV) ;   

extern  uint32_t  StartOffset ;           // 起转时的相偏移
extern  uint32_t  ActiveNum ;             // 换相少于此,函数BLDC_Add_Duty()不改变相偏移            
//======================================================================================= 
extern  uint32_t          *pPhaseEnd ;    // 转动数据最后一相, pPhased大于此相回到第一相
extern  uint32_t           PeriodMax  ;   // 周期最大值, 或周期上限值
extern  uint32_t           PhaseAngle ;   // 过0点到换相的延时,1表示延后约1度  
extern  uint32_t           PeriodShrink ; // 赋值32时,起步阶段周期不变
extern  uint32_t           PhaseOffset ;  // 输出相偏移,输出数据是*(pPhase+PhaseOffset)
extern  uint32_t  volatile ZeroDeadline ; // 赋值 TIMER1->DR 会超时中止检过0

// 以下变量,每次换相后更新   
extern  uint32_t *volatile pPhase    ;    // 当前相指针,过0后指向下一相,数据写入PHCHGNXT           
extern  uint32_t  volatile PeriodNow ;    // 当前周期 
extern  uint32_t  volatile StepCount ;    // 过0计数
extern  uint32_t  volatile ZeroTick ;     // 过0时, Timer1的值
//// ====================================================================================
#endif   // __Mini5xxDE_BLDC_H 
