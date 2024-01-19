#ifndef MAX2870_H_INCLUDED
#define MAX2870_H_INCLUDED

void init_MAX2870_double(void);
void send_MAX2870_double(unsigned char addr, unsigned int data_PLL1, unsigned int data_PLL2);
char read_lock_status_MAX2870_SA(char print_result);
char read_lock_status_MAX2870_TG(char print_result);

#endif /* MAX2870_H_INCLUDED */
