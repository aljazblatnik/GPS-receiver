#include "main.h"
#include "stm32g4xx.h"
#include "stm32g441xx.h" // Suggestions
#include "stdint.h"
#include "pinout.h"
#include "delay.h"
#include "usart.h"
#include "spi_comm.h"
#include "shift_reg.h"
#include "max2870.h"
#include "lmx2491.h"
#include "stuW81300.h"
#include "pe43711.h"
#include "frequency_control.h"

// 120 MHz main CPU freq.
// Stevilo tock zaslona = 512 = podatkov = 1024 - implementiraj det. type -  8 bitov
// če ni podatkov vračaj 0.

void init();

/***** GLOBAL VARIABLES *****/
char buffer[20]; //max 20 characters
int buff_index = 0;
int decode_command = 0;
int num_of_points = 3700;
int num_of_points_count = 0;
int data_send = 1;
/*****  END VARIABLES   *****/
int16_t counter = 0;
extern struct shift_reg_srtuct shift_reg;

int main(void){
    int temp = 0;
    init();
    init_delay();
    init_usart();
    init_spi();
    usart_send_string("\n\rINFO:System boot - OK\n\r");
    usart_send_string("INFO:Waiting for regulators to stabilize\n\r");
    delay_ms(100); // Delay for LDO to stabilize
    init_shift_reg();
    usart_send_string("INFO:System comm bus - OK\n\r");

    init_attenuators();
    usart_send_string("INFO:ATTs OK\n\r");

    // PLL start
    usart_send_string("INFO:VCO 2 SA - ");
    init_MAX2870_double();
    temp = read_lock_status_MAX2870_SA(0);
    if(temp){
        usart_send_string("UNLOCKED\n\r");
    }else {
        usart_send_string("OK\n\r");
    }
    usart_send_string("INFO:VCO 2 TG - ");
    temp = read_lock_status_MAX2870_TG(0);
    if(temp){
        usart_send_string("UNLOCKED\n\r");
    }else {
        usart_send_string("OK\n\r");
    }

    init_LMX2491();
    delay_ms(10);
    temp = read_lock_status_LMX2491(1);
    usart_send_string("INFO:VCO 1-1 - ");
    if(temp){
        usart_send_string("UNLOCKED\n\r");
    }else {
        usart_send_string("OK\n\r");
    }

    init_STUW81300();
    delay_ms(10);
    startup_STUW81300();
    delay_ms(10);
    read_lock_status_STUW81300(1);
    temp = read_status_STUW81300(1);
    usart_send_string("INFO:VCO 1-2 - ");
    if(temp){
        usart_send_string("ERROR\n\r");
    }else {
        usart_send_string("OK\n\r");
    }

    //spi_comm_start_frame();

    // System startup
    LED_ON;
    while(1)
    {
        // TEST area
        sweep_init(5000000,1000000,shift_reg);
        delay_ms(10);
        sweep_single();
        delay_ms(10);
        // TEST area

        if(decode_command){
            if(command_decode(buffer)){
            }
            decode_command = 0;
        }

    }
}

void init()
{
    // Power set to Range 1
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN; // enable clock
    PWR->CR1 = PWR_CR1_VOS_0; // Range 1
    while((PWR->SR2 & PWR_SR2_VOSF) != 0); // Wait for range switch
    FLASH->ACR |= FLASH_ACR_LATENCY_3WS; // (3WS - 4 CPU cycles) - default = 1
    PWR->CR5 |= PWR_CR5_R1MODE; // Normal MODE 1
    // Configure PLL to 120 MHz - 10 MHz crystal oscillator
    RCC->CR |= RCC_CR_HSEON; // turn on HSE
    while(!(RCC->CR & RCC_CR_HSERDY)); // Wait to be ready
    RCC->CR &= ~RCC_CR_PLLON; // turn PLL off
    while((RCC->CR & RCC_CR_PLLRDY) != 0); // wait for PLL to become off
    RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSE | RCC_PLLCFGR_PLLN_3 | RCC_PLLCFGR_PLLN_4 | RCC_PLLCFGR_PLLPEN | RCC_PLLCFGR_PLLREN | RCC_PLLCFGR_PLLPDIV_1 | RCC_PLLCFGR_PLLPDIV_2; // M = 1; N = 24; P = 6 on; Q off; R = 2 on; HSE input clock
    RCC->CR |= RCC_CR_PLLON; // turn PLL ON
    while((RCC->CR & RCC_CR_PLLRDY) == 0); // Wait for PLL clock to start
    RCC->CICR |= RCC_CICR_PLLRDYC; // clear PLLRDY Flag
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while(!(RCC->CFGR & RCC_CFGR_SWS_PLL)); // Wait for clock switch to be done
    // GPIO power enable
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN;

    // PORTS SPEED
    GPIOA->OSPEEDR = 0xCFFFFFFF; // PA13 full speed, PA14 low speed - default
    GPIOB->OSPEEDR = 0xFFFFFFFF; // all B ports full speed
    GPIOC->OSPEEDR = 0x00000000; // Port C low speed

    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD15; // no pullup or down on PA15
    GPIOB->PUPDR = 0x00000000; //No pull-up/pull-down on all ports
    GPIOA->MODER = 0x2BFFFFFF; // PA13 and PA14 for SWD on AF0
    GPIOB->MODER = 0xFFFFFFFF; // Turn off default PB3 AF function
    GPIOB->AFR[0] = 0x00000000; // All AF0 as default state

    // MCU-LED
    GPIOA->MODER &= ~GPIO_MODER_MODE15; // clear default state
    GPIOA->MODER |= GPIO_MODER_MODE15_0; // output

    // RS-232
    GPIOB->MODER &= ~(GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0);  // pin setup PB6 - TX | PB7 - RX - alternate function mode
    GPIOB->AFR[0] |= 0x77000000; // alternate function 7 on pin PB6 and PB7

    // SPI-COMM
    GPIOB->MODER &= ~(GPIO_MODER_MODE3_0 | GPIO_MODER_MODE5_0);  // pin setup PB3 - SPI_CLK | PB5 - SPI_MOSI - alternate function mode
    GPIOB->MODER &= ~(GPIO_MODER_MODE4_0);  // PB4 MISO - output
    GPIOB->AFR[0] |= 0x00555000; // alternate function 5

    // Shift registers
    GPIOA->MODER &= ~(GPIO_MODER_MODE6_1); // output
    GPIOB->MODER &= ~(GPIO_MODER_MODE0_1 | GPIO_MODER_MODE1_1 | GPIO_MODER_MODE2_1); // output
    GPIOA->MODER &= ~GPIO_MODER_MODE7; // input
    GPIOB->MODER &= ~GPIO_MODER_MODE10; // input

    // PLL 1-1 - LMX2491
    GPIOA->MODER &= ~(GPIO_MODER_MODE8_1); // output
    GPIOB->MODER &= ~(GPIO_MODER_MODE11_1 | GPIO_MODER_MODE12_1 | GPIO_MODER_MODE13_1 | GPIO_MODER_MODE15_1); // output
    GPIOB->MODER &= ~(GPIO_MODER_MODE14_0 | GPIO_MODER_MODE14_1); // input

    // PLL 1-2 - STuW81300
    GPIOA->MODER &= ~(GPIO_MODER_MODE9_1 | GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1); // output
    GPIOA->MODER &= ~GPIO_MODER_MODE12; // input
}


/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/

void NMI_Handler(void){
    while (1);
}

void HardFault_Handler(void){
    while (1);
}

void BusFault_Handler(void){
    while (1);
}

void UsageFault_Handler(void){
    while (1);
}

void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

void USART1_IRQHandler(void){
    if(USART1->ISR & USART_ISR_RXNE){ // new data in
        unsigned char data = USART1->RDR;
        if(data == 9){
            usart_send_string("\r\nTAB unsupported!");
            data = 13; // force new line
        }
        if((data == 8 || data == 127) && (buff_index > 0)){
            buff_index--;
            USART1->TDR = data;
            return;
        }
        else if((data == 8 || data == 127) && buff_index<=0){
            return;
        }
        if(data>=32 || data == 13){
            USART1->TDR = data; // echo
            buffer[buff_index] = data;
            if(data == 13){
                buffer[buff_index] = 0;
                decode_command = 1;
            }
            else{
                buff_index++;
            }
        }
    }
    else if(USART1->ISR & USART_ISR_ORE){ // overrun error
        USART1->ICR |= USART_ICR_ORECF; // clear overrun error
        // do nothing
    }
}

void SPI1_IRQHandler(void){
    if(SPI1->SR & SPI_SR_TXE){
        //unsigned char data = SPI1->DR;
        //data = SPI1->DR;
        SPI1->CR2 &= ~SPI_CR2_TXEIE; // disable TXE interrupt
        // transmit buffer is at least 1/2 empty. Write up to 11 new data frames
        SPI1->DR = counter;
        counter++;
        if(counter>0XFFF){
            counter = 0;
        }
        SPI1->CR2 |= SPI_CR2_TXEIE; // RE-enable TXE interrupt
    }
}
