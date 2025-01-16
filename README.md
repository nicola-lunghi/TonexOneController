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
 11. [Companion Projects](#companion_projects)
 12. [License](#license)
 13. [Donations](#donations)

## Demonstration Videos <a name="demonstration_videos"></a>
https://youtu.be/j0I5G5-CXfg

### Full Tutorial Video (in Spanish) thanks to Marcelo
https://www.youtube.com/watch?v=qkOs5gk3bcQ

## Meet the Family <a name="meet_family"></a>
This project can run on any of four different hardware platforms, varying in size and cost. All of them are "off-the-shelf" development boards supplied either by the company "Waveshare", or Espressif.
The code could be adapted to run on other brand ESP32-S3 boards, but to make things easy, pre-built releases are provided for the supported modules.
<br>All platforms support Bluetooth, WiFi, wired footswitches, and wired Midi.
- 4.3" LCD board, supporting touch screen and advanced graphics including customisable amp/pedal skins and text
- 1.69" LCD board. Similar to an Apple Watch, this small board displays the preset name and number
- "Zero" board with no display, is the smallest and cheapest option
- "DevKit-C" board with no display
![meet_family](https://github.com/user-attachments/assets/a2f4ad41-d5b8-4b3d-b800-b53b116014e7)


## ⭐ Key Features <a name="key_features"></a>
The supported features vary a little depending on the chosen hardware platform.
- LCD display with capactive touch screen ("4.3B" model)
- LCD display ("1.69" model)
- Screen displays the name and number of the current preset ("4.3B" and "1.69" models.)
- The User can select an amplifier or pedal skin and also add descriptive text ("4.3B" model)
- Use of simple dual footswitches to select next/previous preset (all platforms)
- Use of 4 buttons to select a preset via a banked system, or directly via binary inputs (all platforms except for the "4.3B")
- Bluetooth Client support. Use of the "M-Vave Chocolate" bluetooth Midi footswitch device to switch presets (4 buttons, bank up/down)
- Other Bluetooth Midi controllers should be also supported, via the "custom name" option. Refer to [Web Configuration](WebConfiguration.md)
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
 
### Hardware platforms without Display
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
V1.0.5.2:
- Added support for ESP "Devkit-C" board
- Added support for the onboard RGB led on the Zero and the Devkit-C. Three green flashes shown on boot.
- Added support for four wired footswitches. Footswitch mode can be set to one of three different modes, using the web config
- Fixed some build warnings

V1.0.4.2:
- Added support for 1.69" version
- New web configuration system
- Unified the BT client/server versions and renamed the modes to Central/Peripheral
- Added custom name text for connecting to other BT peripherals
- Full support for Wired Midi on all platforms
- Updated build system so all platforms use their own build directory and SDK config files
- Created sub-projects so platform can be selected straight from VS Code
- Added Python script to automate creation of release Zip files
- Memory optimisations and latency reduction/performance tuning

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

## Companion Projects <a name="companion_projects"></a>
The MiniMidi project is a small, low cost bluetooth Midi peripheral device, similar to the M-Vave Chocolate pedal.
<br>It can run two or four footswitches, and control the Tonex pedal (or other compatible devices).
Refer to [https://github.com/Builty/TonexOneController/blob/main/minimidi/source](https://github.com/Builty/TonexOneController/blob/main/minimidi/source)

## ©️ License <a name="license"></a>
The Tonex One Controller is under the Apache 2.0 license. It is free for both research and commercial use cases.
<br>However, if you are stealing this work and commercialising it, you are a bad person and you should feel bad.
<br>As per the terms of the Apache license, you are also required to provide "attribution" if you use any parts of the project (link to this project from your project.)

## Donations <a name="donations"></a>
Donations help fund the purchase of new equipment to use in development and testing.<br>
[Donate via Paypal](https://www.paypal.com/donate/?business=RTRMTZA7L7KPC&no_recurring=0&currency_code=AUD)
<br><br>
![QR code](https://github.com/user-attachments/assets/331a7b08-e877-49a4-9d27-2b19a2ff762d)

    
 
