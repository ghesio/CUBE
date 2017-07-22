# CUBE

## What it is

The CUBE is an IOT device with programmable 6 axis gyroscope/accelerometer, programmable LED lights, IR sensor and Bluetooth Low Energy connection.

It's based on Arduino/Genuino 101, featuring a 3D printed and laser cut shell custom designed.

The challenge of this project was to build a device with programmable functions, capable of working on different contexts, aimed at developers who can develop their own use and improvements for the CUBE.

The main idea is to use this as a music controller: based on gyroscope movements and native applications API, you can control the music you are playing; you can change the song you are listening to by rotating it, you can shuffle the music by shaking it.

It’s possible to use it as a volume controller, like a potentiometer: by rotating it with one side laying on the desk, you can increase or decrease music volume.

It will have an infrared sensor to play/stop music by swiping the hand over it.

## Features

- BLE enable
- Battery powered
- 6-Axis gyroscope
- Distance measure
- 4 LEDs acting as output/information

## Functions:

- Able to recognize rotations (clockwise and counterclockwise only, right now)
- Able to recognize swipes of the hand
- Able to recognize the heading of the cube: you can get the angle of the CUBE
- Able to recognize on which side the CUBE is on

## BLE Reference

### Mode control, read/write
*uuid: 988dbab9-a657-45fe-80ef-9f9bed761947, read/write*

This service let choose in which mode you want to use the CUBE

0: Stand-by

1: rotation recognition

2: Swipe recognition

3: Potentiometer mode

4: Face recognition

### Rotations, read/write/notify
*uuid: 988dbab9-a657-45fe-80ef-9f9bed761948, read/write/notify*

This service keeps tracks of the rotations.
When the value is 0, no rotation happened, when it's 1 a left rotation happened, when it's 2 a right rotation happened.
Agter every rotation is necessary to write 0 before reading another.

NOTE: rotations are recognized only clockwise/counterclockwise in the direction indicated in the CUBE.

### LED, read/write
*uuid: 988dbab9-a657-45fe-80ef-9f9bed761949, read/write*

With this service it's possible to control the LEDs.

Write as value a 16-byte char array, where (the number reported here are the index of the array):
- 0: led number (possible value 3,5,6,9)
- 1-3 red component of the color (these 3 bytes represents a number between 0 and 255)
- 4-6 green component of the color (these 3 bytes represents a number between 0 and 255)
- 7-9 blue component of the color (these 3 bytes represents a number between 0 and 255)
- 10-12 white component of the color (these 3 bytes represents a number between 0 and 255)
- 13-15 brigthness (these 3 bytes represents a number between 0 and 255)

Examples:
- 3255000000000050: led #3 turned on, red with brigthness 50
- 3000000000000000: led #3 turned off
- 5000255000000255: led #5 turned on, green with max brigthness


### Swipe, read/write/notify
*uuid: 988dbab9-a657-45fe-80ef-9f9bed761950*

This service keeps track of the swipes.
When the value becomes 1, a swipe happened.
It's needed then to write 0 again in the charachteristic.

### Potenziometro, read/notify
*uuid: 988dbab9-a657-45fe-80ef-9f9bed761951*

This service advertise the CUBE angle.
It's only necessary to read the charachteristic value.
Note: there is a drifting in the PID controlling the gyroscope of about 2 degrees every half and hour.

### Riconoscimento della faccia

Based on which face the CUBE lies on, it's possible to set a different function.
Note: the active face is always the one where you can see the sticker with the number
Note: To use this mode is necessary to do a rotation before in mode 1.

### Final note

If the CUBE starts working random

## Future works / full develompent

My work on this, due to time available is over.

You can get the full story behind the develompent here: https://goo.gl/xEmkgw

Future improvements:
- Shake recognition
- Develop a real application as music controller
- Recognize all rotations

## Presentation video

https://vimeo.com/226481945

## Credits

A special thanks goes to Patrizia Marti and Matteo Sirizzotti.
A special thanks also goes to Elis Taflaj for filming and editing this video.

## License

Copyright (c) 2017 Nicola Giannelli

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
