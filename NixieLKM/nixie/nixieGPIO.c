#include "nixieGPIO.h"

int transfer_spi(int input, int size){
    int i = 0;

    gpio_direction_output(PD14, LOW);

    while(i < size) {
        gpio_direction_output(PD15, (input >> i) & 0x00000001);
        gpio_direction_output(PD14, HIGH);
        gpio_direction_output(PD14, LOW);    
        i++;
    }
    return 0;
}

int init_gpio(void){

    gpio_request(PD14, "data");
    gpio_request(PD15, "clock");
    gpio_direction_output(PD14, LOW);
    gpio_direction_output(PD15, LOW);

    transfer_spi(0x0000, 16);
    transfer_spi(0x0000, 16);

    return 0;
}


int close_gpio(void){

    gpio_direction_output(PD14, LOW);
    gpio_direction_output(PD15, LOW);
    gpio_free(PD14);
    gpio_free(PD15);    

    return 0;
}

void set_dot(int on){
    // if(on == 1){
    //     gpio_direction_output(PC3, HIGH); 
    // } else {
    //     gpio_direction_output(PC3, LOW); 
    // }
}