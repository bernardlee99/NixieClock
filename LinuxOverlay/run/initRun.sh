modprobe nixie

ifconfig wlan0 up
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_init.conf
dhcpcd wlan0

echo "Done!"


echo "static domain_name_servers= 8.8.8.8" > /etc/dhcpcd.conf

