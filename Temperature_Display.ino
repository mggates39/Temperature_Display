#include <Wire.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
#include <Adafruit_SSD1306.h>
#include <RTCZero.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_AM2320 am2320 = Adafruit_AM2320(&Wire);

int displayMode = 0;
unsigned long lastSensorReadTime = 0; // the last time the sensor was read
unsigned long sensorReadDelay = 2000; // how lont to wait between sensor reads

float temperature = 0;
float humidity = 0;

#define CHOOSE_DISPLAY_PIN 2

int buttonState;            // the current reading from the input pin
int lastButtonState = HIGH; // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers

/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

/* Change these values to set the current initial date */
const byte day = 15;
const byte month = 6;
const byte year = 15;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup()
{
  Serial.begin(115200);
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }

  Serial.println("Adafruit AM2320 Basic Test");
  am2320.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // Address 0x3C for 128x32
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();

  display.cp437(true); // Use full 256 char 'Code Page 437' font

  rtc.begin(); // initialize RTC

  // Set the time
  rtc.setHours(hours);
  rtc.setMinutes(minutes);
  rtc.setSeconds(seconds);

  // Set the date
  rtc.setDay(day);
  rtc.setMonth(month);
  rtc.setYear(year);

  // Setup the CHOOSE_DISPLAY_PIN button input pin
  pinMode(CHOOSE_DISPLAY_PIN, INPUT_PULLUP);

  delay(2000);         // Pause for 2 seconds

  // Do the initial sensor read
  readSensor();
}

void loop()
{
  // Clear the buffer
  display.clearDisplay();

  display.setCursor(0, 0);
  display.setTextSize(3); // Draw 3X scale text
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  int reading = digitalRead(CHOOSE_DISPLAY_PIN);

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState)
  {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState)
    {
      buttonState = reading;

      // only toggle the mode if the new button state is LOW
      if (buttonState == LOW)
      {
        displayMode++;
        if (displayMode > 3)
        {
          displayMode = 0;
        }
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

  // Is it time to read the sensor again?
  if ((millis() - lastSensorReadTime) > sensorReadDelay)
  {
    readSensor();
  }

  switch (displayMode)
  {
    case 1: // Centigrade Temp
      display.print(temperature);
      display.write(9);
      display.println("C");
      break;
    case 2: // Farenheight Temp
      display.print(((temperature * 9.0) / 5.0) + 32.0);
      display.write(9);
      display.println("F");
      break;
    case 3:
      display.print(humidity);
      display.println(" %");
      break;
    default:
      displayTime();
  }
  display.display();
}

void readSensor()
{
  temperature = am2320.readTemperature();
  humidity = am2320.readHumidity();

  if (isnan(temperature) || isnan(humidity))
  {
    resetFunc();
  }

  lastSensorReadTime = millis();
}

void displayTime()
{
  display.print(" ");
  display2digits(rtc.getHours());
  if ((rtc.getSeconds() % 2) == 0)
  {
    display.print(":");
  } else {
    display.print(" ");
  }
  display2digits(rtc.getMinutes());
}

void display2digits(int number)
{
  if (number < 10)
  {
    display.print("0");
  }
  display.print(number);
}
