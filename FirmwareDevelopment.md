# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espressif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)

# Development Info
The code is written in C, for the Espressif ESP-IDF development environment version 5.0.2, and using the FreeRTOS operating system.
The LVGL library is used as the graphics engine.

## Task overview:
- Control task is the co-ordinator for all other tasks
- Display task handles the LCD display and touch screen
- Midi Control task handles Bleutooth link to Midi pedals
- USB comms task handles the USB host
- USB Tonex One handles the comms to the Tonex One pedal

## Building Custom sources
Building the application requires some skill and patience.
- Follow the instructions at https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3B#ESP-IDF to install VS Code and ESP-IDF V5.02
- Open the project Source folder using VS Code
- Follow the instructions at https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3B#Modify_COM_Port to select the correct comm port for your Waveshare board, and compile the app

## Menu Config options
Use the Menu Config system to select which components of the Controller you wish to enable.
![image](https://github.com/user-attachments/assets/593d48fb-aeea-4b20-87c7-dc9212952213)

There are also a few settings that need changing between the Waveshare board with display and the Zero without a display, mainly due to the headless board having 4 MB flash versus 8MB on the display board:
### Partition Table
- Display board: "custom partition table csv"
- No Display board: "Factory app, two OTA definitions"

### Serial Flasher Config
Enable Octal Flash:
- Display board: "ticked"
- No Display board: "not ticked"

Flash Spi Mode:
- Display board: "OPI" (Octal mode)
- No Display board: "QIO" (Quad mode)

Flash Size:
- No Display board: "8MB"
- No Display board: "4MB"
 
