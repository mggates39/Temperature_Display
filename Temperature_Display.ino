#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>
#include <Adafruit_SSD1306.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <RTCZero.h>
#include "MyDelay.h"
#include <AceButton.h>
using namespace ace_button;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_AM2320 am2320 = Adafruit_AM2320(&Wire);

int displayMode = -1;

float temperature = 0;
float humidity = 0;

#define CHOOSE_DISPLAY_PIN 2

// Need a UDP socket to access the NTP service
WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
NTPClient timeClient(ntpUDP, "pool.ntp.org", -14400, 60000);

/* Create an rtc object */
RTCZero rtc;

const char ssid[] = "************";      //  your network SSID (name)
const char pass[] = "***********";       // your network password

const char sensorName[] = "Outside 1";
const byte sensorId = 1;


// Initialize the Wifi client library
WiFiClient client;

// Automatically uses the default ButtonConfig. We will configure this later
// using AceButton::init() method in setup() below.
AceButton button;

void handleButtonEvent(AceButton*, uint8_t, uint8_t);
void readSensor();
void dimDisplay();
void updateTimeFromNTP();

myDelay readSensorDelay(2000, readSensor);          // 2 second delay for reading sensor
myDelay dimDisplayDelay(30000, dimDisplay);         // Dim the display after 30 seconds of inactivity
myDelay updateRTCDelay(600000, updateTimeFromNTP);  // Update the RTC every 10 minutes

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup()
{
  Serial.begin(115200);
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }

  Serial.println("Adafruit AM2320 Remote Themometer");
  am2320.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // Address 0x3C for 128x32 and 128x64
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500);

  // Clear the buffer
  display.clearDisplay();

  display.setCursor(0, 0);

  display.cp437(true); // Use full 256 char 'Code Page 437' font
  display.setTextSize(1); // Draw normal scale text
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(true);

  connectToAP();    // connect the board to the access point
  printWifiStatus();

  timeClient.begin();
  timeClient.forceUpdate();

  rtc.begin();
  if (! rtc.isConfigured()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Couldn't find RTC");
    display.display();
    Serial.println("Couldn't find RTC");
    while (1);
  }
  updateTimeFromNTP();
 
  // Setup the CHOOSE_DISPLAY_PIN button input pin
  pinMode(CHOOSE_DISPLAY_PIN, INPUT_PULLUP);

  // We use the AceButton::init() method here instead of using the constructor
  // to show an alternative. Using init() allows the configuration of the
  // hardware pin and the button to be placed closer to each other.
  button.init(CHOOSE_DISPLAY_PIN, LOW);

  // Configure the ButtonConfig with the event handler, and enable the click event
  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleButtonEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2); // Draw 2X scale text
  display.println(sensorName); 
  display.display();

  delay(2000);         // Pause for 2 seconds

  // Do the initial sensor read
  readSensor();

  // Start all the delay timers
  readSensorDelay.start();
  dimDisplayDelay.start();
  updateRTCDelay.start();
}

void loop()
{
  timeClient.update();
  updateRTCDelay.update();
  readSensorDelay.update();
  dimDisplayDelay.update();
 
  button.check();

  displayTheData();
}

void dimDisplay()
{
  display.dim(1);
  dimDisplayDelay.stop();
}

void lightDisplay()
{
  display.dim(0);
  dimDisplayDelay.start();
}

void displayTheData()
{
  // Clear the buffer
  display.clearDisplay();
  display.setTextWrap(false);

  display.setCursor(0, 0);

#if SCREEN_HEIGHT == 32
  display.setTextSize(3); // Draw 3X scale text
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
    case 0:
      displayTime();
      break;
    default:
      display.println(sensorName); 
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
    default:
      display.setTextSize(2);
      display.print(sensorName); 
      display.setTextSize(3);
      display.println("");
  }
  display.print(humidity);
  display.println(" %");
#endif
  display.display();
}

// The event handler for the AceButton.
void handleButtonEvent(AceButton* /* button */, uint8_t eventType,
    uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("handleButtonEvent(): eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  switch (eventType) {
    case AceButton::kEventReleased:
        displayMode++;
        if (displayMode > 3)
        {
          displayMode = 0;
        }
      break;
    case AceButton::kEventPressed:
        lightDisplay();
        
      break;
  }
}

void readSensor()
{
  temperature = am2320.readTemperature();
  humidity = am2320.readHumidity();

  if (isnan(temperature) || isnan(humidity))
  {
    resetFunc();
  }

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
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Firmware: ");
  display.println(WiFi.firmwareVersion());

  // print the SSID of the network you're attached to:
  display.print("SSID: ");
  display.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  display.print("IP Address: ");
  display.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  display.print("signal strength (RSSI):");
  display.print(rssi);
  display.println(" dBm");
  display.display();
}

void connectToAP() {
  int i = 0;
  bool connected = false;

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    display.println("WiFi shield not present");
    display.display();
    // don't continue:
    while (true);
  }

  while(!connected) {
    display.clearDisplay();
    display.setCursor(0,0);
    // attempt to connect to Wifi network:
    display.print("Attempting to connect to SSID: ");
    display.print(ssid);
  
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid, pass);
    i = 0;
    
    while ( WiFi.status() != WL_CONNECTED) {
      display.display();
      // wait 1 second for connection:
      delay ( 1000 );
      display.print( "." );
      i++;
      if (i > 20) {
        WiFi.end();
        break;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
    }
  }
  display.println ( "." );
  display.display();
}

void updateTimeFromNTP()
{
  uint32_t epochTime = timeClient.getEpochTime();
  Serial.println(timeClient.getFormattedTime());

  rtc.setEpoch(epochTime);
}
