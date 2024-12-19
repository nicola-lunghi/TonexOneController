# Tonex One Controller: An open-source controller and display interface for the IK Multimedia Tonex One guitar pedal
This project uses a low-cost embedded controller (Espressif ESP32-S3) to form a bridge to the IK Multimedia Tonex One guitar pedal (which does not have native Midi capability.)

# Programming a pre-built firmware release
## Windows (using ESP Flash Tool)
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

## All OS (using Web Browser)
Thanks to user DrD85 for this method.
- Download the release zip file from the Releases folder and unzip it
- Press and hold the "Boot" button on the Waveshare board
- Connect a USB-C cable to the Waveshare board and a PC
- After the USB connection, release the Boot button
- Open a web browser (use Chrome or Edge) and go to the address "https://esp.huhn.me/"
- Press the Connect button
- A list of serial ports is shown. Look for the port that your ESP32-S3 is using. It may be called "USB JTAG/serial debug unit", or something similar (it may vary with operating system.) Select the port
![image](https://github.com/user-attachments/assets/59729ba8-67f1-4716-9685-93286365d6a3)
- To program the Waveshare Zero, there are 3 files that need to be uploaded. For the Waveshare 4.3B and 1.69, there are 4 files that need to be uploaded. The needed files can be found in the location you unzipped the release to, in the "bin" folder
- Enter the 3 or 4 addresses, and browse to the files in the bin folder, such that the screen looks like the below applicable screen shots
- Once this is set, press the Program button
- The Output windows should show the progress. Once completed, unplug the USB cable, and then power the board normally
![image](https://github.com/user-attachments/assets/aada9df3-826b-450f-a06c-2f5bab24da5b)
<br>
Settings for the Waveshare 4.3B and 1.69:<br>

![image](https://github.com/user-attachments/assets/d0769e53-13ba-4a98-8fc6-e4d4c8adb4d2)


<br><br>
Settings for the Waveshare Zero:<br>
![image](https://github.com/user-attachments/assets/6314a2e5-b9b3-4454-b90d-5dfd13fee6fb)






