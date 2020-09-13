#include "nixieGPIO.h"


int init_gpio(void){

    gpio_request(PC3, "dot");
    gpio_request(PC8, "data");
    gpio_request(PC11, "clock");
    gpio_direction_output(PC3, LOW);
    gpio_direction_output(PC8, LOW);
    gpio_direction_output(PC11, LOW);

    return 0;
}


int transfer_spi(int input, int size){
    int i = 0;

    gpio_direction_output(PC8, LOW);

    while(i < size) {
        gpio_direction_output(PC11, (input >> i) & 0x00000001);
        udelay(5);
        gpio_direction_output(PC8, HIGH);
        udelay(10);
        gpio_direction_output(PC8, LOW);    
        udelay(10);
        i++;
    }
    return 0;
}

int close_gpio(void){

    gpio_direction_output(PC8, LOW);
    gpio_direction_output(PC11, LOW);
    gpio_free(PC8);
    gpio_free(PC11);    

    return 0;
}

void set_dot(int on){
    if(on == 1){
        gpio_direction_output(PC3, HIGH); 
    } else {
        gpio_direction_output(PC3, LOW); 
    }
}