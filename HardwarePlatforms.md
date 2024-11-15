# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
# Hardware Platforms
Two hardware platforms are recommended. 

Other ESP32-S3 platforms could be supported but would require code changes. They would need to meet the minimum requirements:
- Minimum SPI Flash size: 8 MB with display, 4 MB without
- Minimum PSRAM size: 2 MB
- USB OTG port

## Hardware Platform 1: Waveshare 4.3" LCD board (Display supported)
This hardware platform uses this Waveshare 4.3" LCD board.
https://www.waveshare.com/product/esp32-s3-touch-lcd-4.3b.htm?sku=28141

This module provides the microcontroller, power input suitable for 9v DC pedal board use, LCD screen, capacitive touch screen, and dual isolated inputs suitable for momentary foot switches.

### Connections
Note: the controller code relies on the Tonex One pedal being set to Stomp mode. Code is in place to do this automatically, but it seems it may have a bug. Manually enable Stomp mode on your pedal.
- Connect the USB-C port on the Waveshare board to the ToneX One USB-C port
- Optional: connect dual footswitches to the isolated inputs on the Waveshare board. GND to ground. DI0 for footswitch 1. DI1 for footwitch 2. Exact wiring depends on the footswitch but is usually a 6.5mm stereo jack
- Connect 9V DC power supply to the terminals on the Waveshare board. The terminals are screw terminals, so most likely a DC jack to wires will be needed.
- Switch on the power supply
- The Waveshare board USB port will power the Tonex One. Do not connect 9 volts to it!
- Optional: for the Bluetooth Client version of code, switch on a M-Vave Chocolate Midi pedal (https://www.cuvave.com/productinfo/724103.html). After a few seconds it should connect and the Bluetooth icon should change from gray to blue
- Optional: for the Bluetooth Server version of code, the controller will be available as a peripheral for you to connect to via a Bluetooth Midi device. The Bluetooth icon should change from gray to blue when connected.

![image](https://github.com/user-attachments/assets/68c643bd-fe83-4243-ab36-a437c5339e7d)



# Hardware Platform 2: Waveshare ESP32-S3 Zero (no Display support)
This hardware platform uses this Waveshare Zero board.
[https://www.waveshare.com/product/esp32-s3-touch-lcd-4.3b.htm?sku=28141](https://www.waveshare.com/product/esp32-s3-zero.htm)

This module is very low cost (around US$6) and does not support an LCD display. It requires a 5 volt DC power supply.
Caution: do not connect a pedalboard 9v! If you do, you will probably blow up both the PCB and your Tonex One!

### Connections
Note: the controller code relies on the Tonex One pedal being set to Stomp mode. Code is in place to do this automatically, but it seems it may have a bug. Manually enable Stomp mode on your pedal.
- Solder a DC jack to the PCB, as shown below. Note the positive and negative polarity must match your power supply
- Connect the USB-C port on the Waveshare board to the ToneX One USB-C port
- Connect 5 volts DC to the power input jack that you soldered in the first step
- Switch on the power supply
- The Waveshare board USB port will power the Tonex One. Do not connect 9 volts to it!
- Optional: for the Bluetooth Client version of code, switch on a M-Vave Chocolate Midi pedal (https://www.cuvave.com/productinfo/724103.html). After a few seconds it should connect and the Bluetooth icon should change from gray to blue
- Optional: for the Bluetooth Server version of code, the controller will be available as a peripheral for you to connect to via a Bluetooth Midi device. The Bluetooth icon should change from gray to blue when connected.

![wiring_waveshare_zero](https://github.com/user-attachments/assets/cca0ff6e-11d2-4288-8b18-8c177a992fd3)



