while true
do
    TIME_NIXIE_HR=$(($(date +"%H")-5))

    if [ $TIME_NIXIE_HR -lt 0 ]
    then
        TIME_NIXIE_HR=$(($TIME_NIXIE_HR+24))
    fi

    if [ $TIME_NIXIE_HR -lt 10 ]
    then 
        echo 0$TIME_NIXIE_HR:$(date +"%M") > /dev/nixieChar
    else 
        echo $TIME_NIXIE_HR:$(date +"%M") > /dev/nixieChar
    fi
    sleep 1
done