TIMEZONE=-5

# PWM1_LIMITS
PWM1_UPPER_LIMIT=30000
PWM1_LOWER_LIMIT=5000
PWM1_DELTA_H=$((PWM1_UPPER_LIMIT - PWM1_LOWER_LIMIT))
PWM1_GRAD_UP=$((PWM1_DELTA_H/2000))
PWM1_GRAD_DOWN=$((PWM1_DELTA_H/3000))

# PWMCHIP0 @ HV5222
echo 0 > /sys/class/pwm/pwmchip0/export
echo 100000 > /sys/class/pwm/pwmchip0/pwm0/period
echo 100000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable

# PWMCHIP1 @ DOT
echo 0 > /sys/class/pwm/pwmchip1/export
echo 100000 > /sys/class/pwm/pwmchip1/pwm0/period
echo $PWM1_LOWER_LIMIT > /sys/class/pwm/pwmchip1/pwm0/duty_cycle
echo 1 > /sys/class/pwm/pwmchip1/pwm0/enable

TIME_STEP_INDEX=0
CURRENT_PWM1=$PWM1_LOWER_LIMIT
UP_STATE=1
COUNT=10
TIME_NIXIE_MIN_LAST=-1

LDR_ADC_READ=0
LDR_ADC_READ_AVG=0
LDR_ADC_READ_FINAL=0
LDR_ADC_READ_COUNT=0


while true
do
    
    MSEC=$(cat /proc/timer_list | grep "now at ..." | cut -d ' ' -f3 | tail -c 11 | head -c 4)
    MSEC=$(echo $MSEC | awk '{sub(/^0*/,"");}1')

    if  [ $MSEC -lt 2000 ]
    then
        CURRENT_PWM1=$((PWM1_GRAD_UP*MSEC))
        CURRENT_PWM1=$((CURRENT_PWM1+PWM1_LOWER_LIMIT))
    elif [ $MSEC -lt 5000 ]
    then
        CURRENT_PWM1=$((MSEC-2000))
        CURRENT_PWM1=$((PWM1_GRAD_DOWN*CURRENT_PWM1))
        CURRENT_PWM1=$((PWM1_UPPER_LIMIT-CURRENT_PWM1))
    elif [ $MSEC -lt 7000 ]
    then
        CURRENT_PWM1=$((MSEC-5000))
        CURRENT_PWM1=$((PWM1_GRAD_UP*CURRENT_PWM1))
        CURRENT_PWM1=$((CURRENT_PWM1+PWM1_LOWER_LIMIT))
    else
        CURRENT_PWM1=$((MSEC-7000))
        CURRENT_PWM1=$((PWM1_GRAD_DOWN*CURRENT_PWM1))
        CURRENT_PWM1=$((PWM1_UPPER_LIMIT-CURRENT_PWM1))
    fi

    echo $CURRENT_PWM1 > /sys/class/pwm/pwmchip1/pwm0/duty_cycle

    if  [ $COUNT == 25 ]
    then
        TIME_NIXIE_HR=$(($(date +"%H")+TIMEZONE))
        if [ $TIME_NIXIE_HR -lt 0 ]
        then
            TIME_NIXIE_HR=$(($TIME_NIXIE_HR+24))
        fi
        
        TIME_NIXIE_MIN=$(date +"%M")

        if [ $TIME_NIXIE_MIN != $TIME_NIXIE_MIN_LAST ]
        then
            if [ $TIME_NIXIE_HR -lt 10 ]
            then 
                echo 0$TIME_NIXIE_HR:$(date +"%M") > /dev/nixieChar
            else 
                echo $TIME_NIXIE_HR:$(date +"%M") > /dev/nixieChar
            fi
        fi
        
        TIME_NIXIE_MIN_LAST=$TIME_NIXIE_MIN

        COUNT=0
    fi

    COUNT=$((COUNT+1))

    sleep 0.05

done