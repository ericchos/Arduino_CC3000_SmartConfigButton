Adafruit_CC3000_WiFi_PushButton
===============================

An arduino sketch to configure the WiFi settings of the TI CC3000 breakout board from Adafruit.com. The sketch will first check if there are previous WiFi parameters stored in the module. If the WiFi ssid &amp; password is out of date or there is no existing parameters, then the TI smart connect app will run. You can also use a push button connected to pin 2 (interrupt 0) to run the smart config app. You need an iphone/android with the smart config app to change your WiFi ssid and password.
