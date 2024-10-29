# TonexOneController
Embedded controller for the IK Multimedia Tonex One guitar pedal

Note: this project is not endorsed by IK Multimedia. Amplifier skin images copyright is owned by IK Multimedia.

This project allows an Espressif ESP32-S3 microcontroller to interface with the Tonex One guitar pedal.
It supports:
- LCD display with touch screen. Touch screen displays the name and number of the current preset. The User can select an amplifier skin and also add descriptive text 
- Use of simple footswitches to select next/previous presets on the Tonex One
- Use of a "M-Vave Chocolate" bluetooth Midi footswitch device to switch presets
- USB host control of the Tonex pedal

The code is written in C, for the Espressif ESP-IDF development environment.
