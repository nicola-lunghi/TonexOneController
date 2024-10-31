# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espessif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)

**Note: this project is not endorsed by IK Multimedia. Amplifier skin images copyright is owned by IK Multimedia.**

## ⭐ Key Features
- LCD display with capactive touch screen.
- Screen displays the name and number of the current preset.
- The User can select an amplifier skin and also add descriptive text 
- Use of simple dual footswitches to select next/previous preset
- Use of the "M-Vave Chocolate" bluetooth Midi footswitch device to switch presets (4 buttons, bank up/down)
- USB host control of the Tonex pedal

## Hardware Platforms
The recommended hardware platform to use is the Waveshare 4.3" LCD board.
https://www.waveshare.com/product/esp32-s3-touch-lcd-4.3b.htm?sku=28141

This off-the-shelf modules provides the microcontroller, power supply suitable for 9v DC pedal board use, LCD screen, capacitive touch screen, and dual isolated inputs suitable for momentary foot switches.
Other Waveshare moduels in the ESP32-S3 series may also be suitable, but will most likely require some code changes.
For example, the 7" version uses the I2C IO Expander to enable the USB host port.

## Development Info
The code is written in C, for the Espressif ESP-IDF development environment, and using the FreeRTOS operating system.
The LVGL library is used as the graphics engine.

###Task overview:
- Control task is the co-ordinator for all other tasks
- Display task handles the LCD display and touch screen
- Midi Control task handles Bleutooth link to Midi pedals
- USB comms task handles the USB host
- USB Tonex One handles the comms to the Tonex One pedal

## User Interface
The User Interface design was done using Squareline Studio.

UI Design from Squareline Studio:
![image](https://github.com/user-attachments/assets/1246f6e0-0c00-4389-b063-a402bdf45432)


## Operation
### Connections
- Connect the USB-C port on the Waveshare board to the ToneX One USB-C port
- Optional: connect dual footswitches to the isolated inputs on the Waveshare board
- Connect 9V DC power supply to the pins on the Waveshare board
- Switch on the power supply
- The Waveshare board USB port will power the Tonex One. Do not connect 9 volts to it!
- After a few seconds of boot time, the LCD display should now show the description for your Preset number 1 (it will automatically change the Tonex to preset 1 in order to syncronise with it)
- Optional: switch on a M-vave Chocolate Midi pedal. After a few seconds it should connect and the Bluetooth icon should change from gray to blue
- Change presets using one or more of the following methods
  1. Touch screen Next/Previous labels
  2. Dual footswitchs for next/previous preset
  3. M-Vave Chocolate footswitches. Bank 1 does presets 1,2,3,4. Bank 2 does presets 5,6,7,8. Etc.
 
     
 
