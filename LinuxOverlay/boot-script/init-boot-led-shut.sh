INIT_LED_VALUE=0
INIT_BOOT_COMPLETE=0
INIT_BOOT_UP=1

while [ $INIT_LED_VALUE -gt 0 ]
do
    
    INIT_LED_VALUE=$(($INIT_LED_VALUE-1000))

    echo $INIT_LED_VALUE > /sys/class/pwm/pwmchip2/pwm0/duty_cycle

    sleep 0.5
    
done

echo 0 > /sys/class/pwm/pwmchip2/unexport