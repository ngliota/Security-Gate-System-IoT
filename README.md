# Security-Gate-System-IoT
Security parking gate automated using esp32, esp32cam, nodered and parkpow and platerecognizer.com api

First download 2 of the .ino files to upload it and .json for the nodered
Second start the mqtt mosquitto with listener 1883 and anonymous_true in config and start nodered
Third make sure you put the right ip address for mirroring it to the template webserver for ui webserver
Fourth make an account at platerecognizer.com to get the api token then put it to the nodered flows also dont forget to deploy
Fifth good luck using mqtt ive had enough of rc=-2
