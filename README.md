# Photon Controller for LPD6803 LED strips

This project is meant to control a strip of LEDs that use the LPD6803 drivers by using the Particle Photon IOT-connected microcontroller. 

The main code may be seen in /examples/demo/demo.cpp and was adapted by previous demo controllers for Spark Core and Arduino. The main difference here is that instead of running on a fixed loop, the code was written in a state machine approach. Since there is an outside source controlling the pattern of the lights, the code needs to be able to be interrupted easily and change what it is doing.

# Operation

Once the demo project is uploaded to the Particle Photon, a third party webservice needs to be connected to the device. This is to be configured by the user for the given application. At that point, the following functions are made open to be controlled:

-int setMode(String);
-int setColor(String);
-int setSpeed(String);
-int setPulseWidth(String);

## setMode

Possible arguments
-solid [solid color]
-fade [fade in and out]
-pulse [send a pulse of solid color down the strip]
-rainbow [fade rainbow colors along the strip]

## setColor

Possible arguments
-r [for red]
-g [for green]
-b [for blue]
-y [for yellow]
-c [for cyan]
-p [for pink]
-w [for white]
-k [for black]

## setSpeed

Takes an integer. Works best from 100-1000.

The speed is the refresh rate on the colors, so if pulsing, the lights will rotate around the strip at the given speed, or if fading, it will fade in and out at that speed.

## setPulseWidth

Takes an integer, from 1 - the amount of drivers in your strip

Defines how many LEDs are lit up at any given time during the pulse setting. These LEDs will travel along the strip.

# Libraries Used

## [SparkIntervalTimer library](https://github.com/pkourany/SparkIntervalTimer)

A library for using timers on the Spark Core and Photon microcontrollers. 

License and copyright info on the github page, and in the SparkIntervalTimer code

## [LPD6803 Library](https://github.com/adafruit/LPD6803-RGB-Pixels)

An Adafruit library for sending individually addressable color values to strips of LPD6803 LED drivers. 

Slightly edited to fix a hardware issue I was experiencing where the first ~1/3rd of the LED strip was lighting up a different color than the rest of the strip. I opted for the software-solution-to-a-hardware-problem approach, since it seemed easier, and swapped the R-G-B value input for G-B-R for the first section of the LED strip. This should be disabled in the published code here, although if not, disabling it simply requires changing the switchValue variable to 0.

License and copyright info on the github page, and in the LPD6803 code