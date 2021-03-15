#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#define _POSIX_C_SOURCE 200809L

#define DOT_MAX_BRIGHTNESS 40000
// #define DEBUG

#ifndef DEBUG
#define PATH_TIME "/dev/nixieChar"
#define PATH_PWM "/sys/class/pwm/pwmchip2/pwm0/duty_cycle"
#else
#define PATH_TIME "./test.txt"
#define PATH_PWM "./test2.txt"
#endif

time_t t;
struct tm tm;

FILE* nixieChar; 
FILE* pwmDotChar;
char textNixie[5] = "";
char textPWM[5] = "";

int ms; // Milliseconds

void TASK_TIME(void);
void TASK_TIMER(void);
void TASK_DOTPWM(void);
void TASK_DIGITPWM(void);

int TIMER_GetTimer();

int TIME_lastExecution = 0;
int DOTPWM_lastExecution = 0;

int main(void){
    
    TASK_TIMER();
    TIME_lastExecution = TIMER_GetTimer() - 3100;
    DOTPWM_lastExecution = TIMER_GetTimer() - 3100;

    while(1){
        TASK_TIME();
        TASK_TIMER();
        TASK_DOTPWM();
        TASK_DIGITPWM();
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

    sprintf(textNixie, "%02d:%02d", tm.tm_hour + 22, tm.tm_min);
    printf("%02d:%02d\n", tm.tm_hour + 22, tm.tm_min);
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
    if(TIME_lastExecution - ms >= 96000){
        TIME_lastExecution-= 100000;
    }
    if(DOTPWM_lastExecution - ms >= 96000){
        DOTPWM_lastExecution-= 100000;
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
    
    pwmDotChar = fopen(PATH_PWM, "w");

    if(pwmDotChar == NULL){
        printf("Error in Opening Dot PWM Device");
        exit(1);
    }

    sprintf(textPWM, "%d\n", pwmValue);
    fwrite(textPWM, 1, sizeof(textPWM), pwmDotChar);
    fclose(pwmDotChar); 

}

void TASK_DIGITPWM(void){

}

int TIMER_GetTimer(){
    return ms;
}