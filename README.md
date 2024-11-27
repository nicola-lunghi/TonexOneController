# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espressif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)

**Note: this project is not endorsed by IK Multimedia. Amplifier skin images copyright is owned by IK Multimedia.**
**TONEX is a registered trademark of IK Multimedia Production Srl**

## Demonstration Video
https://youtu.be/j0I5G5-CXfg

## ⭐ Key Features
- LCD display with capactive touch screen.
- Screen displays the name and number of the current preset.
- The User can select an amplifier skin and also add descriptive text 
- Use of simple dual footswitches to select next/previous preset
- Bluetooth Client support. Use of the "M-Vave Chocolate" bluetooth Midi footswitch device to switch presets (4 buttons, bank up/down)
- Other Bluetooth Midi controllers should be fairly easy to support with code changes, provided they use the standard Bletooth Midi service and characteristic
- Coming in V1.0.2.2: Bluetooth server support. Pair your phone/tablet with the controller, and send standard Midi program changes, bridged through to the Tonex One pedal (note Server and Client cannot be used simultaneously)
- USB host control of the Tonex pedal
- Serial Midi support
- Menu Config options to disable items like the display, so it could become just a tiny bridge device

## Hardware Platforms
For more information about the hardware platforms, refer to [Hardware Platforms](HardwarePlatforms.md)

## Development Info
For more information about the firmware development and customisation, refer to [Firmware Development](FirmwareDevelopment.md)

## User Interface
For the hardware platform with and LCD display, a User Interface is shown. The User Interface design was done using Squareline Studio: https://squareline.io/

UI Design:
![image](https://github.com/user-attachments/assets/1246f6e0-0c00-4389-b063-a402bdf45432)


## Usage 
### Hardware platform with Display
- Connect power
- After a few seconds of boot time, the LCD display should now show the description for your current Preset
- Change presets using one or more of the following methods
  1. Touch screen Next/Previous labels
  2. Dual footswitchs for next/previous preset
  3. Bluetooth Client mode: M-Vave Chocolate footswitches. Bank 1 does presets 1,2,3,4. Bank 2 does presets 5,6,7,8. Etc.
  4. Bluetooth Server mode: Bluetooth Midi controller to send Program change messages 0 to 19
- The Amplifier skin image is not stored in the Tonex One Pedal, hence this needs to be manually selected
- To select an Amp skin and/or change the description text
  1. Press and hold the Preset name for a few seconds
  2. Navigation arrows will appear next to the amp skin image
  3. Use the left/right arrows to navigate through the available amp skins
  4. Press the description text. A keyboard will appear, allowing text to be entered
  5. Press the green tick image to save the changes. Changes will be saved permanently and remembered when next powered on
 
### Hardware platform without Display
- Connect power
- Change presets using one or more of the following methods
  1. Bluetooth Client mode: M-Vave Chocolate footswitches. Bank 1 does presets 1,2,3,4. Bank 2 does presets 5,6,7,8. Etc.
  2. Bluetooth Server mode: Bluetooth Midi controller to send Program change messages 0 to 19

## Programming a pre-built release
### Common Parts
- Download the release zip file from the Releases folder and unzip it
- Press and hold the "Boot" button on the Waveshare board
- Connect a USB-C cable to the Waveshare board and a PC
- After the USB connection, release the Boot button

### Windows  
- Run the programmer exe on a Windows PC (note: this is provided as a binary package by Espressif Systems, refer to https://www.espressif.com/en/support/download/other-tools)
- Note that Linux and Mac are supported via a Python script. Refer above link.
- Set the Chip Type as "ESP32-S3"
- Set the Work Mode as "Factory"
- Set the Load Mode as "USB" (for devices like the recommended Waveshare module.) Some other PCBs that use a UART instead of the native USB port will need this set to "UART"
- In Download Panel 1, select the Comm port corresponding to your ESP32-S3. This can be determined by checking on the Windows Control Panel, Device Manager, under Ports
- Press the Start button to flash the image into the Waveshare module
- When finished, close the app and disconnect the USB cable (the screen will be blank until the board has been power cycled)
- Follow the Operation instructions

### Linux and MacOS
- Download and install the esptool from https://www.espressif.com/en/support/download/other-tools
- In the extracted release zip file, navigate to the bin folder
- Identify which Com port the board is using (?? how??)
- at a command line, run this command "python -m esptool --chip esp32s3 -b 460800 --before default_reset --after hard_reset --port COM13 write_flash --flash_mode dout --flash_size 8MB --flash_freq 80m 0x0 bootloader.bin 0x100000 TonexController.bin 0x8000 partition-table.bin 0xd000 ota_data_initial.bin" (replace the COM13 with your local value)


## 🙏 Acknowledgement
- [LVGL graphics library] https://lvgl.io/
- [Waveshare board support files]([https://github.com/lifeiteng/vall-e](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3B)) for display and touch screen driver examples
- https://github.com/vit3k/tonex_controller for great work on reverse engineering the Tonex One USB protocol

## Release Notes
V1.0.3.2:
- Changed partition table to fix issues with crashing on boot for some users
- new build type with pedal skin images instead of amp skins
- more efficient handling of skin images
- fixed compatibility issue with Midi BT servers that sent time codes
- support for dual wired footswitches on the Zero
- added support for M-Vave Chocolate Plus footswitch

V1.0.2.2:
- WARNING: these files have been problematic for some users! Please use V1.0.3.2 instead
- Updated to be compatible with Tonex 1.8.0 software
- Support for Waveshare Zero low-cost PCB without display
- Support for Bluetooth Server mode

V1.0.1.2:
- Fixed issue with USB comms. Pedal settings are read, modified (only the preset indexes) and then sent back to pedal
- Note: not compatible with Tonex V1.8.0

V1.0.0.2:
- Initial version
- Caution: this version has an issue with USB. It will overwrite the pedal global settings! use with caution and backup your pedal first
- Note: not compatible with Tonex V1.8.0
  
## ©️ License

The Tonex One Controller is under the Apache 2.0 license. It is free for both research and commercial use cases.
  
     
 
