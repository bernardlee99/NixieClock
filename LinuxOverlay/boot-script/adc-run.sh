LDR_ADC_READ=0
LDR_ADC_READ_AVG=0
LDR_ADC_READ_FINAL=0
LDR_ADC_READ_COUNT=0

PWM_WRITE=100000
TRUE_STATE=1
FALSE_STATE=0
AVERAGE_COUNT=0
CHANGE=0

INIT=1

while true
do
    LDR_ADC_READ=$(cat /sys/bus/iio/devices/iio\:device0/in_voltage0_raw)
    LDR_ADC_READ_AVG=$((LDR_ADC_READ+LDR_ADC_READ_AVG))
    LDR_ADC_READ_COUNT=$((LDR_ADC_READ_COUNT+1))

    if [ $LDR_ADC_READ_COUNT -gt 10 ]
    then
        LDR_ADC_READ_AVG=$((LDR_ADC_READ_AVG/11))
        LDR_ADC_READ_AVG=$((LDR_ADC_READ_AVG-1200))
        TARGET_PWM_WRITE=$((LDR_ADC_READ_AVG*75))
        TARGET_PWM_WRITE=$((TARGET_PWM_WRITE+50000))
        
        
        if [ $TARGET_PWM_WRITE -gt 100000 ]
        then
            TARGET_PWM_WRITE=100000
        elif [ $TARGET_PWM_WRITE -lt 50000 ]
        then
            TARGET_PWM_WRITE=50000
        fi
                
        LDR_ADC_READ_COUNT=0
        LDR_ADC_READ_AVG=0
        AVERAGE_COUNT=$((AVERAGE_COUNT+1))
        INIT=0
    fi

    if [ $INIT -eq $FALSE_STATE ]
    then
        if [ $((PWM_WRITE-TARGET_PWM_WRITE)) -gt 5000 ]
        then 
            echo "TUBE_BRIGHTNESS change to "$TARGET_PWM_WRITE " [DW]"
            CHANGE=1
        elif [ $((PWM_WRITE-TARGET_PWM_WRITE)) -lt -5000 ]
        then
            echo "TUBE_BRIGHTNESS change to "$TARGET_PWM_WRITE " [UP]"
            CHANGE=1
        fi
    fi
    

    if [ $CHANGE -eq $TRUE_STATE  ]
    then
        if [ $((PWM_WRITE-TARGET_PWM_WRITE)) -gt 5000 ]
        then
            PWM_WRITE=$((PWM_WRITE - 5000))
        elif [ $((PWM_WRITE-TARGET_PWM_WRITE)) -lt -5000 ]
        then
            PWM_WRITE=$((PWM_WRITE + 5000))
        else
            PWM_WRITE=$TARGET_PWM_WRITE
            CHANGE=0
        fi
    fi

    if [ $INIT -eq $FALSE_STATE ]
    then
        if [ $CHANGE -eq $FALSE_STATE  ]
        then
            if [ $AVERAGE_COUNT -gt 5 ]
            then
                PWM_WRITE=$TARGET_PWM_WRITE
                AVERAGE_COUNT=0
            fi
        fi
    fi

    sleep 0.1
    
    echo $PWM_WRITE > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
    
done
