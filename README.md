# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espressif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)

**Note: this project is not endorsed by IK Multimedia. Amplifier skin images copyright is owned by IK Multimedia.**
**TONEX is a registered trademark of IK Multimedia Production Srl**

## ⭐ Key Features
- LCD display with capactive touch screen.
- Screen displays the name and number of the current preset.
- The User can select an amplifier skin and also add descriptive text 
- Use of simple dual footswitches to select next/previous preset
- Use of the "M-Vave Chocolate" bluetooth Midi footswitch device to switch presets (4 buttons, bank up/down)
- Other Bluetooth Midi controllers should be fairly easy to support with code changes, provided they use the standard Bletooth Midi service and characteristic
- USB host control of the Tonex pedal
- With simple code changes, it could become just a tiny bridge device without the LCD display and touch screen

## Hardware Platforms
The recommended hardware platform to use is the Waveshare 4.3" LCD board.
https://www.waveshare.com/product/esp32-s3-touch-lcd-4.3b.htm?sku=28141

This off-the-shelf modules provides the microcontroller, power input suitable for 9v DC pedal board use, LCD screen, capacitive touch screen, and dual isolated inputs suitable for momentary foot switches.
Other Waveshare modules in the ESP32-S3 series may also be suitable, but will most likely require some code changes.
For example, the 7" version uses the I2C IO Expander to enable the USB host port.

Other ESP32-S3 development boards could be utilised by changing the source code.
- Minimum SPI Flash size: 8 MB
- Minimum PSRAM size: 2 MB

## Development Info
The code is written in C, for the Espressif ESP-IDF development environment version 5.0.2, and using the FreeRTOS operating system.
The LVGL library is used as the graphics engine.

### Task overview:
- Control task is the co-ordinator for all other tasks
- Display task handles the LCD display and touch screen
- Midi Control task handles Bleutooth link to Midi pedals
- USB comms task handles the USB host
- USB Tonex One handles the comms to the Tonex One pedal

## User Interface
The User Interface design was done using Squareline Studio: https://squareline.io/

UI Design:
![image](https://github.com/user-attachments/assets/1246f6e0-0c00-4389-b063-a402bdf45432)


## Operation
### Connections
- Connect the USB-C port on the Waveshare board to the ToneX One USB-C port
- Optional: connect dual footswitches to the isolated inputs on the Waveshare board
- Connect 9V DC power supply to the terminals on the Waveshare board. The terminals are screw terminals, so most likely a DC jack to wires will be needed.
- Switch on the power supply
- The Waveshare board USB port will power the Tonex One. Do not connect 9 volts to it!
- Optional: switch on a M-Vave Chocolate Midi pedal (https://www.cuvave.com/productinfo/724103.html). After a few seconds it should connect and the Bluetooth icon should change from gray to blue

![image](https://github.com/user-attachments/assets/30d92e47-8d4b-4b66-bce4-e5c8da3cd924)

### Usage
- After a few seconds of boot time, the LCD display should now show the description for your Preset number 1 (it will automatically change the Tonex to preset 1 in order to syncronise with it)
- Change presets using one or more of the following methods
  1. Touch screen Next/Previous labels
  2. Dual footswitchs for next/previous preset
  3. M-Vave Chocolate footswitches. Bank 1 does presets 1,2,3,4. Bank 2 does presets 5,6,7,8. Etc.
- The Ampfilier skin image is not stored in the Tonex One Pedal, hence this needs to be manually selected
- To select an Amp skin and/or change the description text
  1. Press and hold the Preset name for a few seconds
  2. Navigation arrows will appear next to the amp skin image
  3. Use the left/right arrows to navigate through the available amp skins
  4. Press the description text. A keyboard will appear, allowing text to be entered
  5. Press the green tick image to save the changes. Changes will be saved permanently and remembered when next powered on
 

## Programming a pre-built release
- Download the release zip file from the Releases folder and unzip it
- Press and hold the "Boot" button on the Waveshare board
- Connect a USB-C cable to the Waveshare board and a PC
- After the USB connection, release the Boot button
- Run the programmer exe on a Windows PC (note: this is provided as a binary package by Espressif Systems, refer to https://www.espressif.com/en/support/download/other-tools)
- Note that Linux and Mac are supported via a Python script. Refer above link.
- Set the Chip Type as "ESP32-S3"
- Set the Work Mode as "Factory"
- Set the Load Mode as "USB" (for devices like the recommended Waveshare module.) Some other PCBs that use a UART instead of the native USB port will need this set to "UART"
- In Download Panel 1, select the Comm port corresponding to your ESP32-S3. This can be determined by checking on the Windows Control Panel, Device Manager, under Ports
- Press the Start button to flash the image into the Waveshare module
- When finished, close the app and disconnect the USB cable (the screen will be blank until the board has been power cycled)
- Follow the Operation instructions

## Building Custom sources
Building the application requires some skill and patience.
- Follow the instructions at https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3B#ESP-IDF to install VS Code and ESP-IDF V5.02
- Open the project Source folder using VS Code
- Follow the instructions at https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3B#Modify_COM_Port to select the correct comm port for your Waveshare board, and compile the app

## 🙏 Acknowledgement

- [LVGL graphics library] https://lvgl.io/
- [Waveshare board support files]([https://github.com/lifeiteng/vall-e](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3B)) for display and touch screen driver examples

  
## ©️ License

The Tonex One Controller is under the Apache 2.0 license. It is free for both research and commercial use cases.
  
     
 
