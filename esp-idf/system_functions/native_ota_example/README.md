# Native ota with the Node.js server to upload the firmware on the esp32 using the OTA facility

This example contain the native ota facility of the esp32. Just compile the program and upload to the esp32 using the usb.

Then open the version.txt and change the version to higher number,
build the program only.

then run the node js server by giving the command node server.js

* 

Reboot the esp32, it will automatically find the updated firmware file into the module. After updating the module esp32 will reboot.


