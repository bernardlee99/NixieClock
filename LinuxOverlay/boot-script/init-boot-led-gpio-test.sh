INIT_LED_VALUE=0
INIT_BOOT_COMPLETE=0
INIT_BOOT_UP=1
INIT_BOOT_FLASH_COUNT=0

echo 0 > /sys/class/pwm/pwmchip2/export
echo 1000000 > /sys/class/pwm/pwmchip2/pwm0/period
echo $INIT_LED_VALUE > /sys/class/pwm/pwmchip2/pwm0/duty_cycle
echo 1 > /sys/class/pwm/pwmchip2/pwm0/enable

while [ $INIT_BOOT_FLASH_COUNT -lt 3 ]
do
    
    if [[ $INIT_BOOT_UP == 1 ]]
    then
        INIT_LED_VALUE=$(($INIT_LED_VALUE+5000))
    else 
        INIT_LED_VALUE=$(($INIT_LED_VALUE-5000))
    fi
    

    if [[ $INIT_LED_VALUE -gt 999999 && $INIT_BOOT_UP == 1 ]]
    then
        INIT_BOOT_UP=0
    elif [[ $INIT_LED_VALUE -lt 1 && $INIT_BOOT_UP == 0 ]]
    then
        INIT_BOOT_UP=1
        INIT_BOOT_FLASH_COUNT=$(($INIT_BOOT_FLASH_COUNT+1))
    fi

    echo $INIT_LED_VALUE > /sys/class/pwm/pwmchip2/pwm0/duty_cycle
    
done