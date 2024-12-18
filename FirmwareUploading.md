# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espressif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)

# Programming a pre-built firmware release
## Windows (Recommended)
- Download the release zip file from the Releases folder and unzip it
- Open the Windows "Control Centre", "Device Manager", and expand the "Ports" section
- Press and hold the "Boot" button on the Waveshare board
- Connect a USB-C cable to the Waveshare board and a PC
- After the USB connection, release the Boot button
- A new USB serial port should appear. Note down the "COM" number
 
![image](https://github.com/user-attachments/assets/9e7511eb-041e-4aaa-8d95-a3f4d841d678)

- Run the programmer exe on a Windows PC (note: this is provided as a binary package by Espressif Systems, refer to https://www.espressif.com/en/support/download/other-tools)
- Set the Chip Type as "ESP32-S3"
- Set the Work Mode as "Factory"
- Set the Load Mode as "USB"

![image](https://github.com/user-attachments/assets/0c16f2bd-18be-4011-906d-448e4f1dd384)

- In Download Panel 1, select the Comm port corresponding to your ESP32-S3, determined in the prior steps
- Press the Start button to flash the image into the Waveshare module
- When finished, close the app and disconnect the USB cable (the screen will be blank until the board has been power cycled)
- All done!

![image](https://github.com/user-attachments/assets/e2e21f46-1d3a-4eec-aee9-25de87c072c7)

## Linux and MacOS
- Download the release zip file from the Releases folder and unzip it
- Press and hold the "Boot" button on the Waveshare board
- Connect a USB-C cable to the Waveshare board and a PC
- After the USB connection, release the Boot button
- Download and install the esptool from https://www.espressif.com/en/support/download/other-tools
- From the extracted release zip file, navigate to the bin folder
- Identify which Com port the board is using (?? how??)
- At a command line, run this command "python -m esptool --chip esp32s3 -b 460800 --before default_reset --after hard_reset --port COM13 write_flash --flash_mode dout --flash_size 8MB --flash_freq 80m 0x0 bootloader.bin 0x10000 TonexController.bin 0x8000 partition-table.bin 0xd000 ota_data_initial.bin" (replace the COM13 with your local value)
