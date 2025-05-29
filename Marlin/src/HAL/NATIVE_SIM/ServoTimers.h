
#define _useTimer1
//#define _useTimer2
//#define _useTimer3
//#define _useTimer4
//#define _useTimer5

#define TRIM_DURATION             2   // compensation ticks to trim adjust for digitalWrite delays
#define SERVO_TIMER_PRESCALER     2   // timer prescaler

/*
  TC0, chan 0 => TC0_Handler
  TC0, chan 1 => TC1_Handler
  TC0, chan 2 => TC2_Handler
  TC1, chan 0 => TC3_Handler
  TC1, chan 1 => TC4_Handler
  TC1, chan 2 => TC5_Handler
  TC2, chan 0 => TC6_Handler
  TC2, chan 1 => TC7_Handler
  TC2, chan 2 => TC8_Handler
 */

#ifdef _useTimer1
  #define TC_FOR_TIMER1       TC1
  #define CHANNEL_FOR_TIMER1  0
  #define ID_TC_FOR_TIMER1    ID_TC3
  #define IRQn_FOR_TIMER1     TC3_IRQn
  #define HANDLER_FOR_TIMER1  TC3_Handler
#endif
#ifdef _useTimer2
  #define TC_FOR_TIMER2       TC1
  #define CHANNEL_FOR_TIMER2  1
  #define ID_TC_FOR_TIMER2    ID_TC4
  #define IRQn_FOR_TIMER2     TC4_IRQn
  #define HANDLER_FOR_TIMER2  TC4_Handler
#endif
#ifdef _useTimer3
  #define TC_FOR_TIMER3       TC1
  #define CHANNEL_FOR_TIMER3  2
  #define ID_TC_FOR_TIMER3    ID_TC5
  #define IRQn_FOR_TIMER3     TC5_IRQn
  #define HANDLER_FOR_TIMER3  TC5_Handler
#endif
#ifdef _useTimer4
  #define TC_FOR_TIMER4       TC0
  #define CHANNEL_FOR_TIMER4  2
  #define ID_TC_FOR_TIMER4    ID_TC2
  #define IRQn_FOR_TIMER4     TC2_IRQn
  #define HANDLER_FOR_TIMER4  TC2_Handler
#endif
#ifdef _useTimer5
  #define TC_FOR_TIMER5       TC0
  #define CHANNEL_FOR_TIMER5  0
  #define ID_TC_FOR_TIMER5    ID_TC0
  #define IRQn_FOR_TIMER5     TC0_IRQn
  #define HANDLER_FOR_TIMER5  TC0_Handler
#endif

typedef enum : unsigned char {
  #ifdef _useTimer1
    _timer1,
  #endif
  #ifdef _useTimer2
    _timer2,
  #endif
  #ifdef _useTimer3
    _timer3,
  #endif
  #ifdef _useTimer4
    _timer4,
  #endif
  #ifdef _useTimer5
    _timer5,
  #endif
  _Nbr_16timers
} timer16_Sequence_t;
