# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espressif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)
<br>It allows selection of the 20 different presets in the pedal, by any or all of touch screen, wired footswitches, bluetooth footswitches, bluetooth servers, and midi programs.
<br>A variety of hardware is supported, from a $6 board with no display, up to a $44 board with a 4.3" touch screen LCD and a pretty graphical user interface.

**Note: this project is not endorsed by IK Multimedia. Amplifier skin images copyright is owned by IK Multimedia.**
**TONEX is a registered trademark of IK Multimedia Production Srl**

# Table of Contents
 1. [Demonstration Videos](#demonstration_videos)
 2. [Meet the Family](#meet_family)
 3. [Key Features](#key_features)
 4. [Hardware Platforms and Wiring Diagrams](#hardware_platforms)
 5. [Uploading/Programming Firmware Releases](#firmware_uploading)
 6. [Configuration and Settings](#config)
 7. [Usage Instructions](#usage_instructions)
 8. [Firmware Development Information](#development_info)
 9. [Acknowledgements](#acknowledgements)
 10. [Firmware Release Notes](#release_notes)
 11. [License](#license)
 12. [Donations](#donations)

## Demonstration Videos <a name="demonstration_videos"></a>
https://youtu.be/j0I5G5-CXfg

### Full Tutorial Video (in Spanish) thanks to Marcelo
https://www.youtube.com/watch?v=qkOs5gk3bcQ

## Meet the Family <a name="meet_family"></a>
This project can run on any of three different hardware platforms, varying in size and cost. All of them are "off-the-shelf" development boards supplied by the company "Waveshare."
The code could be adapted to run on other brand ESP32-S3 boards, but to make things easy, pre-built releases are provided for the Waveshare modules.
<br>All platforms support Bluetooth, WiFi, wired footswitches, and wired Midi.
- 4.3" LCD board, supporting touch screen and advanced graphics including customisable amp/pedal skins and text
- 1.69" LCD board. Similar to an Apple Watch, this small board displays the preset name and number
- "Zero" board with no display, is the smallest and cheapest option
![meet_family](https://github.com/user-attachments/assets/96e540f4-02d9-4f90-9323-bfc03e967f16)

## ⭐ Key Features <a name="key_features"></a>
The supported features vary a little depending on the chosen hardware platform.
- LCD display with capactive touch screen ("4.3B" model)
- LCD display ("1.69" model)
- Screen displays the name and number of the current preset ("4.3B" and "1.69" models.)
- The User can select an amplifier or pedal skin and also add descriptive text ("4.3B" model)
- Use of simple dual footswitches to select next/previous preset
- Bluetooth Client support. Use of the "M-Vave Chocolate" bluetooth Midi footswitch device to switch presets (4 buttons, bank up/down)
- Other Bluetooth Midi controllers should be fairly easy to support with code changes, provided they use the standard Bletooth Midi service and characteristic
- Bluetooth server support. Pair your phone/tablet with the controller, and send standard Midi program changes, bridged through to the Tonex One pedal (note Server and Client cannot be used simultaneously)
- USB host control of the Tonex pedal
- Wired/Serial Midi support

## Hardware Platforms and Wiring <a name="hardware_platforms"></a>
For more information about the hardware platforms, refer to [Hardware Platforms](HardwarePlatforms.md)

## Uploading/Programming Firmware Releases <a name="firmware_uploading"></a>
For more information about uploading firmware to the boards, refer to [Firmware Uploading](FirmwareUploading.md)

## Configuration and Settings <a name="config"></a>
For more information about changing configuration and settings, for example to change the Midi channel, refer to [Web Configuration](WebConfiguration.md)

## Usage Instructions <a name="usage_instructions"></a>
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

## Firmware Development Infomation <a name="development_info"></a>
For more information about the firmware development and customisation, refer to [Firmware Development](FirmwareDevelopment.md)

## 🙏 Acknowledgements <a name="acknowledgements"></a>
- [Waveshare board support files]([https://github.com/lifeiteng/vall-e](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3B)) for display and touch screen driver examples
- https://github.com/vit3k/tonex_controller for great work on reverse engineering the Tonex One USB protocol

## Firmware Release Notes <a name="release_notes"></a>
V1.0.3.2:
- NOTE: 1.69" version is not supported in this release! 
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
  
## ©️ License <a name="license"></a>
The Tonex One Controller is under the Apache 2.0 license. It is free for both research and commercial use cases.
<br>However, if you are stealing this work and commercialising it, you are a bad person and you should feel bad.

## Donations <a name="donations"></a>
Donations help fund the purchase of new equipment to use in development and testing.<br>
[Donate via Paypal](https://www.paypal.com/donate/?business=RTRMTZA7L7KPC&no_recurring=0&currency_code=AUD)
<br><br>
![QR code](https://github.com/user-attachments/assets/331a7b08-e877-49a4-9d27-2b19a2ff762d)

    
 
