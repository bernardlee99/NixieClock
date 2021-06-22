#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define _POSIX_C_SOURCE 200809L

#define DOT_MAX_BRIGHTNESS 40000
#define DIGIT_MAX_BRIGHTNESS 80000
// #define DEBUG
#define DELTA 4000

/*
~/Desktop/newFS2/NUC970_Buildroot/output/host/usr/bin/arm-linux-gcc \
    -o ~/Desktop/newFS2/NUC970_Buildroot/overlay/usr/bin/nixieChar  \
    ~/Desktop/nixieMain.c 
*/

#ifndef DEBUG
#define PATH_TIME "/dev/nixieChar"
#define PATH_PWM_DOT "/sys/class/pwm/pwmchip2/pwm0/duty_cycle"
#define PATH_PWM_DIGIT "/sys/class/pwm/pwmchip3/pwm0/duty_cycle"
#define PATH_LDR "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#else
#define PATH_TIME "./test.txt"
#define PATH_PWM_DOT "./test2.txt"
#define PATH_PWM_DIGIT "./test4.txt"
#define PATH_LDR "./test3.txt"
#endif

time_t t;
struct tm tm;

FILE* nixieChar; 
FILE* adcChar;
FILE* pwmDotChar;
FILE* pwmDigitChar;
char textNixie[5] = "";
char textPWMDigit[5] = "";
char textADC[5] = "";
char textPWMDot[5] = "";

int ms; // Milliseconds

void TASK_TIME(void);
void TASK_TIMER(void);
void TASK_DOTPWM(void);
void TASK_DIGITPWM(void);
void TASK_ADCREAD(void);

int TIMER_GetTimer();

int TIME_lastExecution = 0;
int DOTPWM_lastExecution = 0;
int DIGITPWM_lastExecution = 0;
int ADCREAD_lastExecution = 0;

int currentPWMDigit = 0;
int targetPWMDigit = 0;

bool executeDigitPWM = false;

int main(void){
    
    TASK_TIMER();
    TIME_lastExecution = TIMER_GetTimer() - 7000;
    DOTPWM_lastExecution = TIMER_GetTimer() - 7000;
    ADCREAD_lastExecution = TIMER_GetTimer() - 7000;
    DIGITPWM_lastExecution = TIMER_GetTimer() - 7000;

    while(1){
        TASK_TIME();
        TASK_TIMER();
        TASK_DOTPWM();
        TASK_DIGITPWM();
        TASK_ADCREAD();
        usleep(50000);
    }

    return 0;
}

void TASK_TIME(void){

    if(TIMER_GetTimer() - TIME_lastExecution < 1500 ){      
        return;
    }
    TIME_lastExecution = TIMER_GetTimer();

    t = time(NULL);
    tm = *localtime(&t);
    nixieChar = fopen(PATH_TIME, "w");

    if(nixieChar == NULL){
        printf("Error in Opening Nixie Device");
        exit(1);
    }

    sprintf(textNixie, "%02d:%02d", tm.tm_hour, tm.tm_min);
    // printf("%02d:%02d\n", tm.tm_hour, tm.tm_min);
    fwrite(textNixie, 1, sizeof(textNixie), nixieChar);
    fclose(nixieChar);    
    
}

void TASK_TIMER(void){

    int s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec % 100;
    ms = spec.tv_nsec / 1.0e6; // Convert nanoseconds to milliseconds
    ms += (s * 1000);
    if(TIME_lastExecution - ms >= 90000){
        TIME_lastExecution-= 100000;
    }
    if(DOTPWM_lastExecution - ms >= 90000){
        DOTPWM_lastExecution-= 100000;
    }
    if(ADCREAD_lastExecution - ms >= 90000){
        ADCREAD_lastExecution-= 100000;
    }
    if(DIGITPWM_lastExecution - ms >= 90000){
        DIGITPWM_lastExecution-= 100000;
    }

}


void TASK_DOTPWM(void){
    if(TIMER_GetTimer() - DOTPWM_lastExecution < 100){
        return;
    }
    DOTPWM_lastExecution = TIMER_GetTimer();
    int scaledTimer = TIMER_GetTimer() - (TIMER_GetTimer() / 10000) % 10 * 10000;   

    int pwmValue = 0;

    if(scaledTimer <= 1000){
        pwmValue = DOT_MAX_BRIGHTNESS * (scaledTimer/1000.0) + 10000;
    } else if(scaledTimer <= 5000){
        pwmValue = DOT_MAX_BRIGHTNESS - (DOT_MAX_BRIGHTNESS * ((scaledTimer-1000.0)/4000.0)) + 10000;
    } else if(scaledTimer <= 6000){
        pwmValue = DOT_MAX_BRIGHTNESS * ((scaledTimer - 5000.0)/1000.0) + 10000;
    } else {
        pwmValue = DOT_MAX_BRIGHTNESS - (DOT_MAX_BRIGHTNESS * ((scaledTimer-6000.0)/4000.0)) + 10000;
    } 
    
    pwmDotChar = fopen(PATH_PWM_DOT, "w");

    if(pwmDotChar == NULL){
        printf("Error in Opening Dot PWM Device");
        exit(1);
    }

    sprintf(textPWMDot, "%d\n", pwmValue);
    fwrite(textPWMDot, 1, sizeof(textPWMDot), pwmDotChar);
    fclose(pwmDotChar); 

}



void TASK_DIGITPWM(void){

    if(TIMER_GetTimer() - DIGITPWM_lastExecution < 6000 && !executeDigitPWM){
        return;
    }

    DIGITPWM_lastExecution = TIMER_GetTimer();
    
    if((targetPWMDigit - currentPWMDigit) > DELTA || (currentPWMDigit - targetPWMDigit) > DELTA){
        executeDigitPWM = true;
    } else {
        currentPWMDigit = targetPWMDigit;
        executeDigitPWM = false;
    }

    if(targetPWMDigit < currentPWMDigit){
        currentPWMDigit -= 2000;
    } else if(targetPWMDigit > currentPWMDigit){
        currentPWMDigit += 2000;
    } 

    pwmDigitChar = fopen(PATH_PWM_DIGIT, "w");

    if(pwmDigitChar == NULL){
        printf("Error in Opening Digit PWM Device");
        exit(1);
    }
    printf("%d: [PWM]\tCurrent: %d\tTarget: %d\n", TIMER_GetTimer() / 100, currentPWMDigit, targetPWMDigit);

    sprintf(textPWMDigit, "%d\n", currentPWMDigit);
    fwrite(textPWMDigit, 1, sizeof(textPWMDigit), pwmDigitChar);
    
    fclose(pwmDigitChar);  

}

void TASK_ADCREAD(void){
    if(TIMER_GetTimer() - ADCREAD_lastExecution < 2000){
        return;
    }

    ADCREAD_lastExecution = TIMER_GetTimer();

    adcChar = fopen(PATH_LDR, "r");

    if(adcChar == NULL){
        printf("Error in Opening LDR Device");
        exit(1);
    }

    fgets(textADC, 5, (FILE*)adcChar);
    int adcReading = atoi(textADC);

    printf("%d: [ADC]\tRead: %d", TIMER_GetTimer() / 100, adcReading);

    if(adcReading < 1500){
        adcReading = 0;
    } else {
        adcReading -= 1500;
    }    

    fclose(adcChar); 

    targetPWMDigit = DIGIT_MAX_BRIGHTNESS - (DIGIT_MAX_BRIGHTNESS * (float)adcReading/2500.0);
    if(targetPWMDigit < 15000){
        targetPWMDigit = 15000;
    }

    if(targetPWMDigit > DIGIT_MAX_BRIGHTNESS){
        targetPWMDigit = DIGIT_MAX_BRIGHTNESS;
    }

    printf("\tPWM: %d\n", targetPWMDigit);
}

int TIMER_GetTimer(){
    return ms;
}