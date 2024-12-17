# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espressif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)

# Programming a pre-built firmware release
## Common Parts
- Download the release zip file from the Releases folder and unzip it
- Press and hold the "Boot" button on the Waveshare board
- Connect a USB-C cable to the Waveshare board and a PC
- After the USB connection, release the Boot button

## Windows  
- Run the programmer exe on a Windows PC (note: this is provided as a binary package by Espressif Systems, refer to https://www.espressif.com/en/support/download/other-tools)
- Note that Linux and Mac are supported via a Python script. Refer above link.
- Set the Chip Type as "ESP32-S3"
- Set the Work Mode as "Factory"
- Set the Load Mode as "USB" (for devices like the recommended Waveshare module.) Some other PCBs that use a UART instead of the native USB port will need this set to "UART"
- In Download Panel 1, select the Comm port corresponding to your ESP32-S3. This can be determined by checking on the Windows Control Panel, Device Manager, under Ports
- Press the Start button to flash the image into the Waveshare module
- When finished, close the app and disconnect the USB cable (the screen will be blank until the board has been power cycled)
- Follow the Operation instructions

## Linux and MacOS
- Download and install the esptool from https://www.espressif.com/en/support/download/other-tools
- In the extracted release zip file, navigate to the bin folder
- Identify which Com port the board is using (?? how??)
- at a command line, run this command "python -m esptool --chip esp32s3 -b 460800 --before default_reset --after hard_reset --port COM13 write_flash --flash_mode dout --flash_size 8MB --flash_freq 80m 0x0 bootloader.bin 0x10000 TonexController.bin 0x8000 partition-table.bin 0xd000 ota_data_initial.bin" (replace the COM13 with your local value)
