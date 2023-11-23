#ifndef PINOUT_H_INCLUDED
#define PINOUT_H_INCLUDED

// MAIN System
#define LED_ON GPIOA->BSRR|=GPIO_BSRR_BS15     //PA15
#define LED_OFF GPIOA->BSRR|=GPIO_BSRR_BR15    //PA15

// UART
/*
PB6 - TX
PB7 - RX
*/

#endif /* PINOUT_H_INCLUDED */
