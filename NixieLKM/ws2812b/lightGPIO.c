#include "lightGPIO.h"

#define GPIO_BA		0xB0004000
#define GPIO_PIN_DATA_BASE (GPIO_BA+0x800)
#define GPIO_PIN_DATA(port, pin)    (*((volatile uint32_t *)((GPIO_PIN_DATA_BASE+(0x40*(port))) + ((pin)<<2))))

int bit_out(int input){
    if(input == 1){
        // GPIO_PIN_DATA(6,6)=1;
        // ndelay(800);
        // GPIO_PIN_DATA(6,6)=0;
        // ndelay(450);
    } else {
        // GPIO_PIN_DATA(6,6)=1;
        // ndelay(400);
        // GPIO_PIN_DATA(6,6)=0;
        // ndelay(850);
    }
}

int data_out(int r, int g, int b){
    gpio_direction_output(PG6, LOW);

    int i = 0;
    while(i < 8) {
        bit_out((r >> i) & 0x00000001);        
        i++;
    }

    i = 0;
    while(i < 8) {
        bit_out((g >> i) & 0x00000001);        
        i++;
    }
    
    i = 0;
    while(i < 8) {
        bit_out((b >> i) & 0x00000001);        
        i++;
    }
    return 0;
}

int init_gpio(void){

    gpio_request(PG6, "data");
    gpio_direction_output(PG6, LOW);
    
    return 0;
}

int close_gpio(void){

    gpio_direction_output(PG6, LOW);
    gpio_free(PG6);

    return 0;
}