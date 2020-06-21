#include <Wire.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
#include <Adafruit_SSD1306.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
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

WiFiUDP ntpUDP;
// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP);

// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

/* Create an rtc object */
RTCZero rtc;

char ssid[] = "************";      //  your network SSID (name)
char pass[] = "***********";       // your network password
int keyIndex = 0;                  // your network key Index number (needed only for WEP)

// Initialize the Wifi client library
WiFiClient client;

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

  connectToAP();    // connect the board to the access point
  printWifiStatus();

  timeClient.begin();

  timeClient.update();
  timeClient.setTimeOffset(-14400);
  Serial.println(timeClient.getFormattedTime());
  Serial.println(timeClient.getDay());

  // Turn off the WiFi after getting ntp to save power for now.
  // We will actually use the WiFi to send data later, but
  // I am running this on a battery and I need to save power now.
  timeClient.end();
  client.stop();
  WiFi.disconnect();
  WiFi.end();

  rtc.begin();
  rtc.setEpoch(timeClient.getEpochTime());

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

#if SCREEN_HEIGHT == 32
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
#else
  display.setTextSize(2); // Draw 2X scale text
  displayTime();
  display.setCursor(0, 20);
  display.setTextSize(3); // Draw 3X scale text
  switch (displayMode)
  {
    case 1: // Centigrade Temp
    case 3: // Centigrade Temp
      display.print(temperature);
      display.write(9);
      display.println("C");
      break;
    case 0: // Farenheight Temp
    case 2: // Farenheight Temp
      display.print(((temperature * 9.0) / 5.0) + 32.0);
      display.write(9);
      display.println("F");
      break;
  }
  display.print(humidity);
  display.println(" %");
#endif
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
#if SCREEN_HEIGHT == 32
  if ((rtc.getSeconds() % 2) == 0)
  {
    display.print(":");
  } else {
    display.print(" ");
  }
#else
  display.print(":");
#endif
  display2digits(rtc.getMinutes());
#if SCREEN_HEIGHT == 64
  display.print(":");
  display2digits(rtc.getSeconds());
#endif
}

void display2digits(int number)
{
  if (number < 10)
  {
    display.print("0");
  }
  display.print(number);
}

void printWifiStatus() {
  Serial.print("Firmware: ");
  Serial.println(WiFi.firmwareVersion());

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void connectToAP() {
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, pass);

  while ( WiFi.status() != WL_CONNECTED) {
    // wait 1/2 second for connection:
    delay ( 500 );
    Serial.print( "." );
  }
  Serial.println ( "." );
}
