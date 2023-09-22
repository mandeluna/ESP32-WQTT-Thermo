/**
 * Read the temperature from a 1-Wire bus device and display it in Fahrenheit and Celcius on an LCD device
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include "Adafruit_LiquidCrystal.h"
#include <WiFi.h>
#include <PubSubClient.h>

// 1-Wire data pin
#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 9

/* ESP32 Connections:
 * 5V to 5V pin
 * GND to GND pin
 * CLK to SCL (GPIO 22)
 * DAT to SDA (GPIO 21)
 */
// Connect via i2c, default address #0 (A0-A2 not jumpered)
Adafruit_LiquidCrystal lcd(0);

// Setup one instance to communicate with all OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses (could be an arbitrary number, limited by hardware)
DeviceAddress temperatureSensor;

// WiFi SSID & password
const char* ssid = "NSA Surveillance Van";
const char* password = "swordfish";

// MQTT Broker address
IPAddress server(192, 168, 0, 155);
const char* mqtt_user = "esp32_thermocouple";
const char* mqtt_pass = "esp32_thermocouple";

WiFiClient wifi;
PubSubClient client(wifi);

void setup() {
  // start serial port
  Serial.begin(115200);
  // configure the LCD's number of columns and rows
  lcd.begin(16, 2);
  // start up the temperature sensors
  setup_onewire();
  // connect to wifi
  setup_wifi();
  // set up MQTT publisher
  setup_mqtt();
}

void setup_mqtt() {
  client.setServer(server, 1883);
  client.setCallback(callback);

  if (!client.connected()) {
    reconnect();
  }
}

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
    Serial.println("connected");
    // Subscribe
    client.subscribe("esp32/output");
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}

void setup_wifi() {
  delay(10);

  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// set up 1-Wire sensors
void setup_onewire() {
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // assign address manually.  the addresses below will beed to be changed
  // to valid device addresses on your bus.  device address can be retrieved
  // by using either oneWire.search(deviceAddress) or individually via
  // sensors.getAddress(deviceAddress, index)
  //insideThermometer = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };
  //outsideThermometer   = { 0x28, 0x3F, 0x1C, 0x31, 0x2, 0x0, 0x0, 0x2 };

  // search for devices on the bus and assign based on an index.  ideally,
  // you would do this to initially discover addresses on the bus and then 
  // use those addresses and manually assign them (see above) once you know 
  // the devices on your bus (and assuming they don't change).
  // 
  // method 1: by index
  if (!sensors.getAddress(temperatureSensor, 0)) Serial.println("Unable to find address for Device 0"); 

  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices, 
  // or you have already retrieved all of them.  It might be a good idea to 
  // check the CRC to make sure you didn't get garbage.  The order is 
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to insideThermometer
  //if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");
  // assigns the seconds address found to outsideThermometer
  //if (!oneWire.search(outsideThermometer)) Serial.println("Unable to find address for outsideThermometer");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(temperatureSensor);
  Serial.println();

  // set the resolution to 9 bit
  sensors.setResolution(temperatureSensor, TEMPERATURE_PRECISION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(temperatureSensor), DEC); 
  Serial.println();
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void clearLCD(int line) {
  lcd.setCursor(0, line);
  for (int i=0; i < 16; i++) lcd.print(" ");
  lcd.setCursor(0, line);
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);

  // Print the temperature in C and F on the top line
  clearLCD(0);
  lcd.print(tempC, 1);
  lcd.print("C ");
  lcd.print(DallasTemperature::toFahrenheit(tempC), 1);
  lcd.print("F");
}

// function to print the temperature for a device
void publishTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);

  char tempString[64];
  sprintf(tempString, "{\"temperature\":%5.1f}", tempC);
  client.publish("sensor/bbq", tempString);
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();    
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  // Serial.print("Device Address: ");
  // printAddress(deviceAddress);
  printTemperature(deviceAddress);
}

void loop() {
  // display the temperature data locally
  sensors.requestTemperatures();
  printData(temperatureSensor);
  // publish the temperature data & check for messages
  client.loop();
  publishTemperature(temperatureSensor);

  if (!client.connected()) {
    reconnect();
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  // if (String(topic) == "esp32/output") {
  //   Serial.print("Changing output to ");
  //   if(messageTemp == "on"){
  //     Serial.println("on");
  //     digitalWrite(ledPin, HIGH);
  //   }
  //   else if(messageTemp == "off"){
  //     Serial.println("off");
  //     digitalWrite(ledPin, LOW);
  //   }
  // }
}