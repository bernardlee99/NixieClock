LDR_ADC_READ=0
LDR_ADC_READ_AVG=0
LDR_ADC_READ_FINAL=0
LDR_ADC_READ_COUNT=999

echo 0 > /sys/class/pwm/pwmchip2/export
echo 1000000 > /sys/class/pwm/pwmchip2/pwm0/period
echo 1 > /sys/class/pwm/pwmchip2/pwm0/enable

while true
do
    LDR_ADC_READ=$(cat /sys/bus/iio/devices/iio\:device0/in_voltage0_raw)
    LDR_ADC_READ_AVG=$(($LDR_ADC_READ+$LDR_ADC_READ_AVG))
    LDR_ADC_READ_COUNT=$(($LDR_ADC_READ_COUNT+1))

    if [[ $LDR_ADC_READ_COUNT -gt 1000 ]]
    then
        PWM_WRITE=$(($LDR_ADC_READ_AVG*243/1000))
        if [[ $PWM_WRITE -gt 1000000 ]]
        then
            $PWM_WRITE = 1000000
        fi
        echo $PWM_WRITE > /sys/class/pwm/pwmchip2/pwm0/duty_cycle
        LDR_ADC_READ_COUNT=0
        LDR_ADC_READ_AVG=0
    fi
    
done