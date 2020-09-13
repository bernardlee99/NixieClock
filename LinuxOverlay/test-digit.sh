HR1=0
HR0=0

MIN1=0
MIN0=0

CONT=1

while [ $CONT -gt 0  ]
do
    echo $HR1$HR0 $MIN1$MIN0 > /dev/nixieChar
    MIN0=$((MIN0 + 1))

    if [ $MIN0 -gt 9 ]
    then
        
        MIN1=$((MIN1 + 1))
        MIN0=0
    fi

    if [ $MIN1 -gt 5 ]
    then
        
        HR0=$((HR0 + 1))
        MIN1=0
    fi

    if [ $HR0 -gt 9 ]
    then
        
        HR1=$((HR1 + 1))
        HR0=0
    fi

    if [ $HR1 -gt 1 ]
    then
        if [ $HR0 -gt 3 ]
        then
            CONT=0
        fi
    fi

    sleep 0.01

done