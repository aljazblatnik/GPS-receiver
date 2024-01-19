#include "stm32g4xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usart.h"
#include "delay.h"
#include "lmx2491.h"
#include "stuW81300.h"
#include "frequency_control.h"
#include "pe43711.h"

extern int buff_index;
extern int num_of_points;
extern int data_send;

void init_usart(void) {
    RCC->APB2ENR |= (RCC_APB2ENR_USART1EN); // clock
    USART1->PRESC = USART_PRESC_PRESCALER_2 | USART_PRESC_PRESCALER_0; // /10 freq 120 MHz/10 = 12MHz

    //USART1->BRR = 104; // 115.25 KBps
    USART1->BRR = 1250; // 9600 bits/s - 1250
    USART1->CR1 |= USART_CR1_RE; // receive enable
    USART1->CR1 |= USART_CR1_TE; // transmit enable
    //USART1->CR1 |= USART_CR1_RTOIE; // timeout interrupt enable
    //USART1->CR2 |= USART_CR2_RTOEN; // Receiver timeout enable
    USART1->RTOR = 0xFFFFFF; // timeout value
    USART1->CR1 |= USART_CR1_UE; // usart enable

    // receive interrupt setup
    USART1->CR1 |= USART_CR1_RXNEIE;

    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn,2);   //enable interrupt

    /* kako uporabljati prekinitvene funkcije:
    (USART1->ISR & USART_ISR_RXNE) // data ready to be read
    USART1->RDR; // receive register
    USART1->TDR; // transmit register
    */
}

void usart_send_string(char *str) {
    int i = 0;
    while(str[i] != '\0') {
        USART1->TDR = str[i];
        i++;
        while(!(USART1->ISR & USART_ISR_TC));
    }
}

void usart_send_byte(unsigned char byte) {
    USART1->TDR = byte;
    while(!(USART1->ISR & USART_ISR_TC));
}

char* int_to_dec_string(int i, char b[]) {
    char const digit[] = "0123456789";
    char* p = b;
    // negative number
    if(i<0) {
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do { //Move to where representation ends
        ++p;
        shifter = shifter/10;
    } while(shifter);
    *p = '\0';
    do { //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    } while(i);
    return b;
}

char* int_to_hex_string(unsigned int i, char b[]) {
    char const digit[] = "0123456789ABCDEF";
    char* p = b;
    unsigned int shifter = i;
    do { //Move to where representation ends
        ++p;
        shifter = shifter/16;
    } while(shifter);
    *p = '\0';
    do { //Move back, inserting digits as u go
        *--p = digit[i%16];
        i = i/16;
    } while(i);
    return b;
}

int string_to_int(char *str, int length) {
    // dolzina je stevilo znakov, ki jih vzame v obdelavo, vkljucno s predznakom!
    unsigned int result = 0;
    char const digit[] = "0123456789";
    int i = 0;
    int sign = 1;

    // check length
    for(int n=0; n<length; n++){
        if(str[n] == '\n') {
            length = n + 1; // set new length
            break;
        }
    }

    if(str[0] == '-') {
        sign = -1;
        length--;
        i = 1;
    } else if(str[0] == '+') {
        sign = 1;
        length--;
        i = 1;
    }
    while(length) {
        int n;
        for(n=0; digit[n]; n++) {
            if(digit[n] == str[i]) break;
        }
        result *= 10;
        result += n;
        i++;
        length--;
    }
    result *= sign;
    return result;
}

char* int_to_bin_string(unsigned int i, char b[]) {
    char const digit[] = "01";
    char* p = b;
    /*
    // negative number
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    */
    int shifter = i;
    do { //Move to where representation ends
        ++p;
        shifter = shifter/2;
    } while(shifter);
    *p = '\0';
    do { //Move back, inserting digits as u go
        *--p = digit[i%2];
        i = i/2;
    } while(i);
    return b;
}

int register_bin_to_int(char *str) {
    // podatek brez markerja 0b!
    unsigned int result = 0;
    int i = 0;
    while(str[i] != '\0') {
        int n = 0;
        if(str[i] == '1') {
            n = 1;
        }
        result *= 2;
        result += n;
        i++;
    }
    return result;
}

int register_hex_to_int(char *str) {
    // podatek brez markerja 0b!
    unsigned int result = 0;
    char const digit[] = "0123456789ABCDEF";
    int i = 0;
    while(str[i] != '\0') {
        int n;
        for(n=0; digit[n]; n++) {
            if(digit[n] == str[i]) {
                break;
            }
        }
        result *= 16;
        result += n;
        i++;
    }
    return result;
}

int compare_string(char *first, char *second) { // substring
    while (*first == *second) {
        if (*first == '\0' || *second == '\0')
            break;
        first++;
        second++;
    }

    if (*second == '\0')
        return 0;
    else
        return -1;
}

int command_decode(char *buffer) {
    if(buff_index == 0) {
        usart_send_string("System: OK\r\n");
        return 0;
    }
    if(compare_string(buffer, "help") == 0) {
        // lahko posljemo seznam ukazov
        usart_send_string("START - SPI transmission start\r\nSTOP - SPI transmission stop\r\nPOINTS xxxxxx - Number of points with leading zeros\r\nhelp - This help\r\n\r\n");
    } else if(compare_string(buffer, "START") == 0) {
        data_send = 1;
        usart_send_string("OK\r\n");
    } else if(compare_string(buffer, "STOP") == 0) {
        data_send = 0;
        usart_send_string("OK\r\n");
    } else if(compare_string(buffer, "POINTS ") == 0) {
        int result = string_to_int(&buffer[7],6);
        num_of_points = result;
        usart_send_string("OK\r\n");
    } else if(compare_string(buffer, "R LO1-1 -all") == 0) {
        char str[34];
        int a = 141;
        int b = 0;
        while(b <= a) {
            int result = read_LMX2491(b);
            int_to_hex_string(b, str);
            usart_send_string("\r\nR");
            usart_send_string(str);
            int_to_hex_string(result, str);
            usart_send_string(": 0x");
            usart_send_string(str);
            int_to_bin_string(result, str);
            usart_send_string(" - 0b");
            usart_send_string(str);
            b++;
        }
        usart_send_string("\n\r");
    } else if(compare_string(buffer, "R LO1-2 -all") == 0) {
        char str[34];
        int a = 11;
        int b = 0;
        while(b <= a) {
            int result = read_STUW81300(b);
            int_to_hex_string(b, str);
            usart_send_string("\r\nR");
            usart_send_string(str);
            int_to_hex_string(result, str);
            usart_send_string(": 0x");
            usart_send_string(str);
            int_to_bin_string(result, str);
            usart_send_string(" - 0b");
            usart_send_string(str);
            b++;
        }
        usart_send_string("\n\r");
    } else if(compare_string(buffer, "R") == 0) {
        // get register number
        int a = 1;
        char str[12];
        while(buffer[a] != ' ') {
            str[a-1] = buffer[a];
            a++;
        }
        str[a-1] = '\0';
        unsigned int register_num = string_to_int(str, 4);
        if(compare_string(&buffer[a+1], "?") == 0) {
            // get data
            char str_a[34];
            unsigned int result = read_STUW81300(register_num);
            int_to_hex_string(register_num, str_a);
            usart_send_string("\r\nR");
            usart_send_string(str_a);
            int_to_hex_string(result, str_a);
            usart_send_string(": 0x");
            usart_send_string(str_a);
            int_to_bin_string(result, str_a);
            usart_send_string(" - 0b");
            usart_send_string(str_a);
        } else {
            // send data
            unsigned int register_data;
            if(compare_string(&buffer[a+2], "b") == 0) {
                register_data = register_bin_to_int(&buffer[a+3]);
                usart_send_string("\r\nBin - working ...");
                char str_a[17];
                int_to_bin_string(register_data, str_a);
                usart_send_string(" Decoded: 0b");
                usart_send_string(str_a);
                int_to_hex_string(register_num, str_a);
                usart_send_string(" R");
                usart_send_string(str_a);
            } else if(compare_string(&buffer[a+2], "x") == 0) {
                register_data = register_hex_to_int(&buffer[a+3]);
                usart_send_string("\r\nHex - working ...");
            } else {
                register_data = string_to_int(&buffer[a+1], 5);
                usart_send_string("\r\nDec - working ...");
            }
            if(register_data > 0x7FFFFFF) {
                usart_send_string("\r\nErr - Input number too large!");
            } else {
                send_STUW81300(register_num,register_data);
                usart_send_string("\r\nSending - OK");
            }
        }
    } else if(compare_string(buffer, "CW1 ") == 0) {
        char str_a[10];
        int actual_freq = set_CW_frequency_LMX2491(string_to_int(&buffer[4],6));
        usart_send_string("Actual frequency: ");
        int_to_dec_string(actual_freq,str_a);
        usart_send_string(str_a);
        usart_send_string(" Hz\r\n");
        usart_send_string("OK\r\n");
    } else if(compare_string(buffer, "CW2 ") == 0) {
        char str_a[10];
        int actual_freq = set_CW_frequency_STUW81300(string_to_int(&buffer[4],7));
        usart_send_string("Actual frequency: ");
        int_to_dec_string(actual_freq,str_a);
        usart_send_string(str_a);
        usart_send_string(" kHz\r\n");
        usart_send_string("OK\r\n");
    } else if(compare_string(buffer, "CW ") == 0) {
        char str_a[10];
        int actual_freq = set_CW_frequency(string_to_int(&buffer[3],7));
        usart_send_string("Actual frequency: ");
        int_to_dec_string(actual_freq,str_a);
        usart_send_string(str_a);
        usart_send_string(" kHz\r\n");
        usart_send_string("OK\r\n");
    } else {
        usart_send_string("Unrecognized command. Try help.\r\n");
        buff_index = 0;
        return 0;
    }
    buff_index = 0;
    return 1;
}
