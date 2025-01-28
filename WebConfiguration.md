# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espressif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)

# Configuration and Settings
New in firmware version V1.0.4.2 is the ability to change settings via WiFi/Web browser.

![image](https://github.com/user-attachments/assets/07d6cdf1-bab2-421f-a68a-1e5ec8eb8390)


## Available Settings
### Bluetooth Mode
- Disabled: Bluetooth is totally disabled and non-functional
- Central (default): allows the controller to locate and connect to other peripherals, like the M-Vave Chocolate
- Peripheral: allows the controller to be discovered and connected to by other Central devices (like a Phone or a PC.)

### Bluetooth Devices
- Enable support for the M-Vave Chocolate and Chocolate Plus bluetooth footswitch controllers (default: on)
- Enable support for the X-Vive MD1 Midi bridge device (default: on)
- Enable support for some other Bluetooth Midi peripheral. Enter its device name, and check the checkbox to enable it (default: off)

### Wired Midi
- Enable support for wired Midi. CAUTION: this should only be enabled when the correct hardware has been connected. If not, random preset switching may occur!
- Midi channel: select the desired Midi channel to use with Wired Midi. 

### Preset Twice Toggle
- If this setting is disabled (default), then setting the same preset index multiple times will not have any effect. <br>
- If this setting is enabled, then setting the same preset a second time will set the Tonex pedal to bypass mode. Setting it a third time will exit bypass mode.
This setting is most suited to use with Pedal models, where it could for example enable/disable an overdrive pedal

### Footswitch Mode (added in V1.0.5.1)
This setting controls how directly wired footswitches will function. Note this has nothing to do with Bluetooth footswitch pedals.
- Dual Next/Previous: 2 footswitches that select preset next and previous
- Quad Banked: 4 footswitches. 5 banks of 4 presets are controlled. 1+2 selects down a bank. 3+4 selects up a bank. Single switch selects the preset of ((bank * 4) + switch number)
- Quad Binary: 4 switch inputs, intended for control by relays. The preset selected depends on the binary combination switch inputs, with switch 1 being the least significant bit, and switch 4 being the most significant bit. E.g. switch inputs 1,0,1,1 = preset 11

### Bluetooth Midi Control Change Enable (added in V1.0.6.1)
This setting controls if the Bluetooth Central Midi can accept Control Change commands (e.g. enabling effects and changing parameters.)<br>
This is disabled by default, as the MVave Chocolate pedal, when changing banks, sends a conflicting change that modifies the Tonex parameters.<br>
This setting should not be enabled with a Chocolate pedal that has the default configuration loaded!

### Save and Reboot
The Save Settings and Reboot button must be pressed to save the changes. The controller will reboot.

## Initial Settings
Important note: when this page is loaded, the default settings are shown. It does NOT SHOW the currently selected settings! (Sorry, this is really difficult with the ESP32 HTTP server component.)<br>
If you are unsure what settings you have currently selected, just change them to what you need and save them.

## Entering Settings Mode
The Controller enables a WiFi access point for the first 60 seconds after it is powered on. After 60 seconds, if no device is connected, then WiFi is disabled. This is to ensure it can never interfere with the normal operations, and to ensure some smart audience member doesn't mess around with your device.<br>
If you do connect to the controller, you should either Save Settings and Reboot (which will disconnect your device) or if not, manually disconnect from the controller. If you do not do this, your device will remain connected and WiFi will remain enabled.

<br>
To change settings:
- Reboot the controller
- Within 60 seconds, use a phone or PC to connect to the WiFi device "TonexConfig"
- The password for the network is 12345678
- The controller will automatically supply a network address for your device (DHCP is supported)
- Note: some phones may attempt to use this network for Internet access, which will not be be available. Watch out for any messages asking you to confirm the connection (like the Samsung warning shown below)
- Open a web browser on your device
- In the address bar of the web browser, enter "tonex.local" (without the quotation marks.) You should see the web config screen
- Once you have saved the settings (or if you don't want to change anything) you can close the web browser
<br>
![image](https://github.com/user-attachments/assets/fbd02f79-06c9-44e0-bb0a-ba67c129d41d)

