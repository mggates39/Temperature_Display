#include <Wire.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

#define OVERRIDE 2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_AM2320 am2320 = Adafruit_AM2320(&Wire);
int displayMode = 0;
unsigned long lastSensorReadTime = 0;
unsigned long sensorReadDelay = 2000;

float temperature = 0;
float humidity = 0;

int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit AM2320 Basic Test");
  am2320.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Setup the Override button input pin
  pinMode(OVERRIDE, INPUT_PULLUP);


  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  readSensor();
}

void loop() {
  // Clear the buffer
  display.clearDisplay();

  display.setCursor(0, 0);
  display.setTextSize(3); // Draw normal-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  int reading = digitalRead(OVERRIDE);

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the mode if the new button state is LOW
      if (buttonState == LOW) {
        displayMode ++;
        if (displayMode > 2) {
          displayMode = 0;
        }
      }
    }
  }


  if ((millis() - lastSensorReadTime) > sensorReadDelay) {
    readSensor();
  }

  switch (displayMode) {
    case 0: // Centigrade Temp
      display.print(temperature); display.println(" C");
      break;
    case 1: // Farenheight Temp
      display.print(((temperature * 9.0) / 5.0) + 32.0); display.println(" F");
      break;
    default:
      display.print(humidity); display.println(" %");
  }
  display.display();
  
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

}

void readSensor()
{
  temperature = am2320.readTemperature();
  humidity = am2320.readHumidity();
  
  lastSensorReadTime = millis();

}
