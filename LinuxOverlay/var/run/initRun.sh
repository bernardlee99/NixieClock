modprobe nixie

ifconfig wlan0 up
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_init.conf
dhcpcd wlan0

echo "nameserver 8.8.8.8" > /etc/resolv.conf

echo "Init script completed!"

