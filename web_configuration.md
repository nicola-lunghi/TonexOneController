# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espressif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)

# Configuration and Settings
New in firmware version V1.0.4.2 is the ability to change settings via WiFi/Web browser.
![image](https://github.com/user-attachments/assets/a4e832b2-340f-4fb4-9565-aa6a7d77863e)

## Available Settings
### Bluetooth Mode
- Disabled: Bluetooth is totally disabled and non-functional
- Central (default): allows the controller to locate and connect to other peripherals, like the M-Vave Chocolate
- Peripheral: allows the controller to be discovered and connected to by other Central devices (like a Phone or a PC.)

### Bluetooth Devices
- Enable support for the M-Vave Chocolate and Chocolate Plus bluetooth footswitch controllers (default: on)
- Enable support for the X-Vive MD1 Midi bridge device (default: on)

### Wired Midi
- Enable support for wired Midi
- Midi channel: select the desired Midi channel to use with Wired Midi. 

### Preset Twice Toggle
- If this setting is disabled (default), then setting the same preset index multiple times will not have any effect. <br>
- If this setting is enabled, then setting the same preset a second time will set the Tonex pedal to bypass mode. Setting it a third time will exit bypass mode.
This setting is most suited to use with Pedal models, where it could for example enable/disable an overdrive pedal

### Save and Reboot
The Save Settings and Reboot button must be pressed to save the changes. The controller will reboot.

## Initial Settings
Important note: when this page is loaded, the default settings are shown. It does NOT SHOW the currently selected settings! (Sorry, this is really difficult with the ESP32 HTTP server component.)<br>
If you are unsure what settings you have currently selected, just change them to what you need and save them.

## Entering Settings Mode
The Controller enables a WiFi access point for the first 60 seconds after it is powered on. After 60 seconds, WiFi is disabled, to ensure it can never interfere with the normal operations, and to ensure some smart audience member doesn't mess around with your device.
<br>
To change settings:
- Reboot the controller
- Within 60 seconds, use a phone or PC to connect to the WiFi device "TonexConfig"
- The password for the network is 12345678
- The controller will automatically supply a network address for your device (DHCP is supported)
- Note: some phones may attempt to use this network for Internet access, which will not be be available. Watch out for any messages asking you to confirm the connection (like the Samsung warning shown below)
- Open a web browser on your device
- In the address bar of the web browser, enter "192.168.4.1" (without the quotation marks.) You should see the web config screen
- Once you have saved the settings (or if you don't want to change anything) you can close the web browser

![image](https://github.com/user-attachments/assets/fbd02f79-06c9-44e0-bb0a-ba67c129d41d)

