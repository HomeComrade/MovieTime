exec 3<> /dev/serial/by-path/platform-bcm2708_usb-usb-0:1.3.4:1.0
sleep 5
TZ='Europe/Tallinn' date +T%s\n > /dev/serial/by-path/platform-bcm2708_usb-usb-0:1.3.4:1.0
exec 3>&-
