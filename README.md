# Temperature Sensor
An electronic temperature sensor with display

## Description 
This demo project is just a simple keypad with display to drive a lock relay.  It also has
a buzzer to generate tones for keypress, success and failure.

The pass codde is hard coded in the application.

## Parts 
- 1 Arduino Nano 33 IOT
- 1 SSD1306 0.91" 128x32 OLED screen w/I2C
- 1 AM2320 Temperature and Humidity Sensor w/I2C
- 1 Momentary Push Button
- 2 10K Ohm resistors
- 1 Breadboard
- Assorted Jumper wires

## Libraries
- Wire
- Adafruit_Sensor
- Adafruit_AM2320
- Adafruit_SSD1306

## Wiring
First we will wire up the power rails of the breadboard
- Wire one of the Arduino Ground (GND) Pins to the ground or (-) rail of the breadboard.
- Wire the Arduino 3.3 voilt Pin to the power or (+) rail of the breadboard.

Next  we will wire the I2C bus.  It will require the 10K pullup resistors.
- Wire one end of the first 10K (R1) to Arduino Pin SCL - A5
- Wire one end of the second 10K (R2) to Arduino Pin SDA - A4

Now we are read
Wire the AM2330 temperature sensor to the Arduino.  I have found that it works best if it is the first device in the I2C bus.

| I2C Pin | Usage | Breadboard position |
| ------- | ----- | ----------- |
| Vcc | 3.3 volts | Power Rail
| GND | Ground | Ground Rail |
| SCL | Serial Clock | The other end of R1 |
| SDA | Serial Data | The other end of R2 |

Wire the OLED Display to the Arduino

| I2C Pin | Usage | Breadboard positionn |
| ------- | ----- | ----------- |
| Vcc | 3.3 volts | Power Rail
| GND | Ground | Ground Rail |
| SCL | Serial Clock | The AM2320 SCL Pin |
| SDA | Serial Data | The AM2320 SDA Pin |

Wire the momentary Pushbutton

| Pin | Usage | Arduino Pin |
| ------- | ----- | ----------- |
| 1,2 | Signal | D2 |
| 3,4 | Ground | Ground Rail |

## Program

## References
- Adafruit display and sensor libraries can be found in the IDE Library Manager
