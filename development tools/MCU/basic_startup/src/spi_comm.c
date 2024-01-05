#include "stm32g4xx.h"
#include "stm32g441xx.h" // Suggestions
#include "stdint.h"
#include "pinout.h"

void init_spi(void){
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // Enable SPI clock

    SPI1->CR1 |= SPI_CR1_LSBFIRST | SPI_CR1_MSTR | SPI_CR1_BR_2; // Clock 120 MHz / 32 = 3.75 MHz
    SPI1->CR1 |= SPI_CR1_SSM; // Enable software slave control - slave not active - logical high
    SPI1->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2; // 8 bit data

    SPI1->CR1 |= SPI_CR1_SPE; // enable spi
}

void spi_send_data(unsigned char data){
    SPI1->DR = data;
    while(!(SPI1->SR & SPI_SR_TXE)); // wait for transmit
}
