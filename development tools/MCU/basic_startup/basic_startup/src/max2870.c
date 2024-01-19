#include "main.h"
#include "stm32g4xx.h"
#include "stm32g441xx.h" // Suggestions
#include "stdint.h"
#include "pinout.h"
#include "delay.h"
#include "shift_reg.h"
#include "structs.h"
#include "usart.h"

extern struct shift_reg_srtuct shift_reg;

struct MAX2870_reg {
    unsigned int R0;
    unsigned int R1;
    unsigned int R2;
    unsigned int R3;
    unsigned int R4;
    unsigned int R5;
    unsigned int R6;
    unsigned char R0_adr;
    unsigned char R1_adr;
    unsigned char R2_adr;
    unsigned char R3_adr;
    unsigned char R4_adr;
    unsigned char R5_adr;
    unsigned char R6_adr;
} MAX2870_reg;

struct MAX2870_reg MAX2870_PLL_SA;
struct MAX2870_reg MAX2870_PLL_TG;

void send_MAX2870_double(unsigned char addr, unsigned int data_PLL1, unsigned int data_PLL2){
    data_PLL1 &= 0xFFFFFFF8;
    data_PLL1 |= (addr&0x00000007); // prepare data
    data_PLL2 &= 0xFFFFFFF8;
    data_PLL2 |= (addr&0x00000007); // prepare data

    int i = 32;
    shift_reg.PLL_SA_LE = 0; // LE low
    shift_reg.PLL_TG_LE = 0;
    shift_reg.PLL_SA_DATA = 0;  // data low
    shift_reg.PLL_TG_DATA = 0;
    sendDataToShiftRegisters();

    //send data to MAX2870s
    while(i){
        shift_reg.PLL_SA_CLK = 0;
        shift_reg.PLL_TG_CLK = 0;
        if(data_PLL1 & 0x80000000){
            shift_reg.PLL_SA_DATA = 1;
        }
        else{
            shift_reg.PLL_SA_DATA = 0;
        }
        if(data_PLL2 & 0x80000000){
            shift_reg.PLL_TG_DATA = 1;
        }
        else{
            shift_reg.PLL_TG_DATA = 0;
        }
        sendDataToShiftRegisters();
        data_PLL1 = data_PLL1<<1;
        data_PLL2 = data_PLL2<<1;
        shift_reg.PLL_SA_CLK = 1;
        shift_reg.PLL_TG_CLK = 1;
        sendDataToShiftRegisters();
        i--;
    }
    shift_reg.PLL_SA_CLK = 0;
    shift_reg.PLL_TG_CLK = 0;
    shift_reg.PLL_SA_LE = 1;
    shift_reg.PLL_TG_LE = 1;
    sendDataToShiftRegisters();
    /* - to ne rabis
    shift_reg.PLL_SA_LE = 0;
    shift_reg.PLL_TG_LE = 0;
    sendDataToShiftRegisters();
    */
}

void init_MAX2870_double(void){
    // Register address - SA
    MAX2870_PLL_SA.R0_adr = 0x00;
    MAX2870_PLL_SA.R1_adr = 0x01;
    MAX2870_PLL_SA.R2_adr = 0x02;
    MAX2870_PLL_SA.R3_adr = 0x03;
    MAX2870_PLL_SA.R4_adr = 0x04;
    MAX2870_PLL_SA.R5_adr = 0x05;
    MAX2870_PLL_SA.R6_adr = 0x06;
    // Register address - TG
    MAX2870_PLL_TG.R0_adr = 0x00;
    MAX2870_PLL_TG.R1_adr = 0x01;
    MAX2870_PLL_TG.R2_adr = 0x02;
    MAX2870_PLL_TG.R3_adr = 0x03;
    MAX2870_PLL_TG.R4_adr = 0x04;
    MAX2870_PLL_TG.R5_adr = 0x05;
    MAX2870_PLL_TG.R6_adr = 0x06;

    // Default values - SA
    MAX2870_PLL_SA.R0 = 0x80D38000; // 4230 MHz
    MAX2870_PLL_SA.R1 = 0x80008011;
    MAX2870_PLL_SA.R2 = 0x58011FC2;
    MAX2870_PLL_SA.R3 = 0x0100000B;
    MAX2870_PLL_SA.R4 = 0x608C8234;
    MAX2870_PLL_SA.R5 = 0x01400005;
    MAX2870_PLL_SA.R6 = 0; // read register
    // Default values - TG
    MAX2870_PLL_TG.R0 = 0x80D70000; // 4300 MHz
    MAX2870_PLL_TG.R1 = 0x80008011;
    MAX2870_PLL_TG.R2 = 0x58011FC2;
    MAX2870_PLL_TG.R3 = 0x0100000B;
    MAX2870_PLL_TG.R4 = 0x608C8234;
    MAX2870_PLL_TG.R5 = 0x01400005;
    MAX2870_PLL_TG.R6 = 0; // read register

    send_MAX2870_double(MAX2870_PLL_SA.R5_adr,MAX2870_PLL_SA.R5,MAX2870_PLL_TG.R5);
    delay_ms(20);

    send_MAX2870_double(MAX2870_PLL_SA.R4_adr,MAX2870_PLL_SA.R4,MAX2870_PLL_TG.R4);
    send_MAX2870_double(MAX2870_PLL_SA.R3_adr,MAX2870_PLL_SA.R3,MAX2870_PLL_TG.R3);
    send_MAX2870_double(MAX2870_PLL_SA.R2_adr,MAX2870_PLL_SA.R2,MAX2870_PLL_TG.R2);
    send_MAX2870_double(MAX2870_PLL_SA.R1_adr,MAX2870_PLL_SA.R1,MAX2870_PLL_TG.R1);
    send_MAX2870_double(MAX2870_PLL_SA.R0_adr,MAX2870_PLL_SA.R0,MAX2870_PLL_TG.R0);

    send_MAX2870_double(MAX2870_PLL_SA.R5_adr,MAX2870_PLL_SA.R5,MAX2870_PLL_TG.R5);
    send_MAX2870_double(MAX2870_PLL_SA.R4_adr,MAX2870_PLL_SA.R4,MAX2870_PLL_TG.R4);
    send_MAX2870_double(MAX2870_PLL_SA.R3_adr,MAX2870_PLL_SA.R3,MAX2870_PLL_TG.R3);
    send_MAX2870_double(MAX2870_PLL_SA.R2_adr,MAX2870_PLL_SA.R2,MAX2870_PLL_TG.R2);
    send_MAX2870_double(MAX2870_PLL_SA.R1_adr,MAX2870_PLL_SA.R1,MAX2870_PLL_TG.R1);
    send_MAX2870_double(MAX2870_PLL_SA.R0_adr,MAX2870_PLL_SA.R0,MAX2870_PLL_TG.R0);

    shift_reg.PLL_SA_RF_EN = 1;
    shift_reg.PLL_TG_RF_EN = 1;
    shift_reg.PLL_SA_DATA = 0;
    shift_reg.PLL_TG_DATA = 0;
    sendDataToShiftRegisters();
}

// print_result 1 = true, 0 = false
// output 0 - OK (LOCKED), 1 - ERROR (UNLOCKED)
char read_lock_status_MAX2870_SA(char print_result){
    char output_status = 0;
    if(print_result) usart_send_string("LO2-SA:PLL loop - ");
    if(PLL1_MISO) {
        if(print_result) usart_send_string("LOCKED\n\r");
    } else {
        if(print_result) usart_send_string("UNLOCKED\n\r");
        if(print_result) usart_send_string("ERROR:LO2-SA PLL UNLOCKED\n\r");
        output_status = 1;
    }
    return output_status;
}

// print_result 1 = true, 0 = false
// output 0 - OK (LOCKED), 1 - ERROR (UNLOCKED)
char read_lock_status_MAX2870_TG(char print_result){
    char output_status = 0;
    if(print_result) usart_send_string("LO2-TG:PLL loop - ");
    if(PLL2_MISO) {
        if(print_result) usart_send_string("LOCKED\n\r");
    } else {
        if(print_result) usart_send_string("UNLOCKED\n\r");
        if(print_result) usart_send_string("ERROR:LO2-TG PLL UNLOCKED\n\r");
        output_status = 1;
    }
    return output_status;
}

/*
int MAX2870_set_frequency(int frequency){
    int REG0 = 0; // all 0
    int N = 0;
    int FRAC = 0;
    int DIV = 0;
    int freq_out = 0;

    while(DIV <= 7){
        if(frequency > (3000000>>DIV)) break; // delimo frekvenco vsakic z dve
        DIV++;
    }

    N = (frequency<<DIV)/32000; // prej frekvenco ustrezno mnozimo z izbranim delilnikom, da nastavimo vrednost
    FRAC = (((frequency<<DIV)%32000)*4000)/32000;

    freq_out = ((32000*N)+((FRAC*32000)/4000))>>DIV;

    FRAC = FRAC<<3;
    N = N<<15;
    REG0 |= (FRAC & 0x00007FF8);
    REG0 |= (N & 0x7FFF8000);

    MAX2870_reg.R0 = REG0;
    MAX2870_reg.R4 &= 0b11111111100011111111111111111111; //resetiramo DIVA bite
    MAX2870_reg.R4 |= DIV<<20; // Dodamo DIV

    send_MAX2870_all(MAX2870_reg);

    return freq_out;
}
*/
