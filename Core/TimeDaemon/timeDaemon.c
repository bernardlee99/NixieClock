#include<stdlib.h>
#include<stdio.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

//DOT PWM parameters
#define DOT_PWM_UPPER_LIMIT 30000
#define DOT_PWM_LOWER_LIMIT 5000
#define DOT_PWM_DELTA_UP 12.5
#define DOT_PWM_DELTA_DOWN 8.333

//DIGIT PWM parameters
#define DIGIT_PWM_UPPER_LIMIT 60000
#define DIGIT_PWM_LOWER_LIMIT 30000
#define DIGIT_PWM_DELTA 

//PATH parameters
// #define DEBUG

#ifdef DEBUG
    #define TIME_PATH "time.txt"
    #define DIGIT_PWM_PATH "n_pwm.txt"
    #define DOT_PWM_PATH "d_pwm.txt"
    #define ADC_PATH "adc.txt"
#else
    #define TIME_PATH "/dev/nixieChar"
    #define DIGIT_PWM_PATH "/sys/class/pwm/pwmchip0/pwm0/duty_cycle"
    #define DOT_PWM_PATH "/sys/class/pwm/pwmchip1/pwm0/duty_cycle"
    #define ADC_PATH "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#endif

struct timespec _t;

int millis(){    
    clock_gettime(CLOCK_REALTIME, &_t);
    return (_t.tv_sec*1000 + lround(_t.tv_nsec/1.0e6)) % 10000; 
}

int main(void){

    FILE *timePath; 
    timePath = fopen(TIME_PATH, "w");

    FILE *dotPWMPath; 
    dotPWMPath = fopen(DOT_PWM_PATH, "w");

    FILE *digitPWMPath; 
    digitPWMPath = fopen(DIGIT_PWM_PATH, "w");

    FILE *adcPath; 
    adcPath = fopen(ADC_PATH, "r");

    if(timePath == NULL || dotPWMPath == NULL || digitPWMPath == NULL || adcPath == NULL){
        printf("Time Daemon Initialization Error (fileop error)\n");
        return -1;
    }

    int dot_pwm = 0;
    int micro = 0;

    time_t rawtime;
    struct tm * timeinfo;
    
    char timeData[10];    
    
    char LDR_Reading[10];
    int LDR_ReadingAvg = 0;
    int digitTargetPWM = 0;


    int count = 0;
    
    while(1){
        
        micro = millis();
        
        //DOT PWM Operation
        if(micro <= 2000){
            dot_pwm = DOT_PWM_LOWER_LIMIT + (int)(micro * DOT_PWM_DELTA_UP);    
        } else if(micro <= 5000){
            dot_pwm = DOT_PWM_UPPER_LIMIT - (int)((micro - 2000) * DOT_PWM_DELTA_DOWN);      
        } else if(micro <= 7000){
            dot_pwm = DOT_PWM_LOWER_LIMIT + (int)((micro - 5000) * DOT_PWM_DELTA_UP);    
        } else {
            dot_pwm = DOT_PWM_UPPER_LIMIT - (int)((micro - 7000) * DOT_PWM_DELTA_DOWN);      
        }
        #ifdef DEBUG
        printf("DOT_PWM: %d\n", dot_pwm); 
        #else
        fprintf(dotPWMPath, "%d\n", dot_pwm);
        #endif
        usleep(100000); //Delay 100ms

        //Time Operation
        if(count % 10 == 0){
            time(&rawtime);
            timeinfo = localtime( &rawtime );
            sprintf(&timeData, "%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min);
            #ifdef DEBUG
            printf("TIME: %s\n", timeData);
            #else
            fprintf(timePath, "%s\n", timeData);
            #endif            
        }

        //ADC Reading
        fgets(LDR_Reading, 10, adcPath);
        LDR_ReadingAvg += atoi(LDR_Reading);
        if(count == 50){
            LDR_ReadingAvg = LDR_ReadingAvg / 11.0;
            LDR_ReadingAvg -= 1200;
            digitTargetPWM = LDR_ReadingAvg * 75;
            digitTargetPWM += 50000;

            if(digitTargetPWM > 100000){
                digitTargetPWM = 100000;
            } else if(digitTargetPWM < 50000){
                digitTargetPWM = 50000;
            }
            count = 0;
            LDR_ReadingAvg = 0;
            #ifdef DEBUG
            printf("DIGIT_PWM: %d\n",digitTargetPWM);
            #else
            fprintf(digitPWMPath, "%d\n",digitTargetPWM);
            #endif           
        }

        //Digit PWM Operation 
        

        count++;
    }

    fclose(timePath);
    fclose(dotPWMPath);
    fclose(digitPWMPath);
    fclose(adcPath);

    return 0;
}