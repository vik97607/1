# 1
Simple internet radio with ESP8266 and wifi webconfig

Credits to Earl F. Philhower for the audio player and Manish Sarma for the wifi webconfig

Use Arduino IDE to upload the code.

Install the following libraries:
1.Wifi (1.2.7 at the time of making)

2.ESP8266Audio - by Earl F. Philhower (1.8.1 at the time of making)
  
Use the RX port as sound output, best trough a 1Kohm resistor to the base of an NPN transistor.

Short user manual:

When the board is turned on, it is trying to connect to the 2.4GHz WIFI network with the default settings.
If this is not succesful, it will become an access point with SSID Klubradio - you have to connect with a 
laptop or mobile phone.
You have to be able to connect by typing the address: 192.168.4.1 in a web browser.
You have to write the appropriate SSID and Password in the small box below the list of access points.
The board will restart, connect to the wifi and play Klubradio Hungary. It will remember the credentials forever - or if this is not succesfull, go back to wifi setting mode.


