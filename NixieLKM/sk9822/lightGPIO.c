#include "lightGPIO.h"

void SPI_write(int c){

  int i;
  for(i = 0; i < 8; i++){
    if (!(c&0x80)) {
      gpio_direction_output(PG6, LOW);
    } else {
      gpio_direction_output(PG6, HIGH);
    }

    c <<= 1;
    
    gpio_direction_output(PG7, HIGH);
    gpio_direction_output(PG7, LOW);
  }
  
}

void set_led_brightness(struct cRGB *ledarray, int leds, int brightness){
  int i;
  int *rawarray=(int*)ledarray;
  
  gpio_direction_output(PG7, LOW);
  
  // Start Frame
  SPI_write(0x00);   
  SPI_write(0x00);
  SPI_write(0x00);
  SPI_write(0x00);
 
  for (i=0; i<(leds+leds+leds); i+=3){
    SPI_write(0xe0+brightness);
    SPI_write(rawarray[i+0]);
    SPI_write(rawarray[i+1]);
    SPI_write(rawarray[i+2]);
  }

  // Reset frame
  SPI_write(0x00);  
  SPI_write(0x00);
  SPI_write(0x00);
  SPI_write(0x00);
  
  // End frame: 8+8*(leds >> 4) clock cycles    
  for (i=0; i<leds; i+=16){
    SPI_write(0x00);  
  }

}

void set_led(struct cRGB *ledarray, int leds){
  set_led_brightness(ledarray,leds,150);
}


int init_gpio(void){
    gpio_request(PG6, "data");
    gpio_direction_output(PG6, LOW);

    gpio_request(PG7, "clock");
    gpio_direction_output(PG7, LOW);
    
    return 0;
}


int close_gpio(void){
    gpio_direction_output(PG6, LOW);
    gpio_free(PG6);

    gpio_direction_output(PG7, LOW);
    gpio_free(PG7);

    return 0;
}



void loading(void){
    int i = 0;  

    if(topIndex < GRAD_LENGTH - 1){
        topIndex++;
    } else if(topIndex >= (GRAD_LENGTH - 1) && topIndex < (LENGTH - 1)){
        topIndex++;
        bottomIndex++;
    } else if(topIndex >= (LENGTH - 1) && bottomIndex < (LENGTH - 1)){
        bottomIndex++;
    } else if(bottomIndex >= (LENGTH - 1)){
        topIndex = 0;
        bottomIndex = 0;
    }

    for(i = 0; i < LENGTH; i++){
        led[i].b = 0;
    }

    delta = topIndex - bottomIndex;
    
    if(delta == (GRAD_LENGTH - 1)){
        for(i = 0; i < GRAD_LENGTH; i++){
            led[bottomIndex + i].b = grad[i];
        }
    } else if(bottomIndex == 0 && delta != (GRAD_LENGTH - 1)){
        for(i = 0; i <= delta; i++){
            led[i].b = grad[(GRAD_LENGTH - 1) - (delta - i)];
        }
    } else if(topIndex == (LENGTH - 1) && delta != (GRAD_LENGTH - 1)){
        for(i = 0; i <= delta; i++){
            led[bottomIndex + i].b = grad[i];
        }
    }
    set_led(led, LENGTH);
}

void loading2(void){
    int i = 0;
    if(loadingProgressDirection == UP){
        for(i = 0; i < loadingProgress; i++){
            led[i].b = 50;
        }  
    } else {
        for(i = 0; i < LENGTH; i++){
            led[i].b = loadingPulse;
        } 
    }

    if(loadingProgress > (LENGTH - 1) && loadingProgressDirection == UP){
        loadingProgressDirection = DOWN;
        loadingPulse = 50;
    } else if(loadingPulse <= 0 && loadingProgressDirection == DOWN){
        loadingProgressDirection = UP;
        loadingProgress = 0;
    }

    if(loadingProgressDirection == UP){
        loadingProgress+=1;
    } else{
        loadingPulse-=5;
    }

    set_led(led, LENGTH);
    
}

void pulsing(void){
    int i = 0;

    for(i = 0; i < LENGTH; i++){
        led[i].r = pulseBrightness;
    }

    if(pulseDirection == UP){
        pulseBrightness+=5;
    }else{
        pulseBrightness-=1;
    }
    
    if(pulseBrightness >= 100){
        pulseDirection = DOWN;
    } else if(pulseBrightness <= 0){
        pulseDirection = UP;
    }

    set_led(led, LENGTH);
  
}

void setAll(int r, int g, int b){
    int i = 0;
    for(i = 0; i < LENGTH; i++){
        led[i].r = r;
        led[i].g = g;
        led[i].b = b;
    }
    set_led(led,LENGTH);
}