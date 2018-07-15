#include <Arduino.h>

// https://github.com/PaulStoffregen/Time
#include <TimeLib.h>

// http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <AccelStepper.h>

// https://github.com/kroimon/Arduino-SerialCommand
#include <SerialCommand.h>

// https://github.com/esp8266/Arduino
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

// https://github.com/bblanchon/ArduinoJson
#include <ArduinoJson.h>


// ###################################################
// ################# INIT ############################
// ###################################################

// ----------------- WIFI ----------------------------

const char* ssid     = "H369AB21CAF";
const char* password = "9DA66C3A229E";

int steps = 45000;
int speed = 5000;
int acceleration = 5000;

bool isEnabled = false;
float amount = 0.75;
int sunriseHour = 8;
int sunriseMinutes = 0;
bool sunHasJustRisen = false;

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

// ----------------- Time ----------------------------

unsigned int timeSyncUDPPort = 2390;      // local port to listen for UDP packets
IPAddress timeServerIP;
// const char* ntpServerName = "pool.ntp.org";
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48;

byte packetBuffer[NTP_PACKET_SIZE];
WiFiUDP udp;

int secondsGtmOffset = 3600; // GTM+2

int prevMinute = 0;

void digitalClockDisplay();

// ----------------- Motor ----------------------------

// Pins on the Huzzah microcontroller
#define motorStepPin 4
#define motorDirPin 5
#define motorOnOffPin 16
#define switchPin 14

// Default settings
#define defaultSpeed 1000
#define defaultAcceleration 1000

AccelStepper motor(AccelStepper::DRIVER, motorStepPin, motorDirPin);
int switchValue = LOW;








// ###################################################
// ################# Motor code ######################
// ###################################################

// ----------------- Helpers -----------------

void enableOutputs() {
  Serial.println("- motor.enableOutputs");  
  motor.enableOutputs();
}

void disableOutputs() {
  Serial.println("- motor.disableOutputs");  
  motor.disableOutputs();
}

void setMaxSpeed(int value) {
  Serial.print("- motor.setMaxSpeed: ");  
  Serial.println(value);  
  motor.setMaxSpeed(value);
}

void setAcceleration(int value) {
  Serial.print("- motor.setAcceleration: ");  
  Serial.println(value);  
  motor.setAcceleration(value);
}

void setCurrentPosition(int value) {
  Serial.print("- motor.setCurrentPosition: ");  
  Serial.println(value);  
  motor.setCurrentPosition(value);
}

void moveTo(int value) {
  Serial.print("- motor.moveTo: ");  
  Serial.println(value);  
  motor.moveTo(value);
}

void stop() {
  Serial.println("- motor.stop");
  motor.stop();
}

void startSunrise() {
  Serial.print("- startSunrise");
  enableOutputs();
  setCurrentPosition(0);
  moveTo(amount * steps);
}

void checkSunRise() {
  if(hour() == sunriseHour && minute() == sunriseMinutes) {
    startSunrise();
  } 
}

// ----------------- Setup -----------------

void setupMotor(){
    Serial.println("- setupMotor");

    pinMode(motorStepPin, OUTPUT);

    motor.setEnablePin(motorOnOffPin);
    disableOutputs();
}

void loopMotor()
{     
  if (motor.distanceToGo() == 0) {
    // Auto turn motor off when it's done rotating
    motor.disableOutputs();
  }

  if (timeStatus() != timeNotSet) {
    // Only once a minute
    if (minute() != prevMinute) {
      prevMinute = minute();
      digitalClockDisplay();
      checkSunRise();
    }
  }

  motor.run();
}







// ###################################################
// ################# Wifi code ######################
// ###################################################



void handleEchoQueryArgumentsAsJSON();
void handleSettings ();
void handleSunrise ();
void handleOpen ();
void handleStop ();

void setupWifi() {
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    // Signal wifi is not connected
    digitalWrite(0, HIGH);
    Serial.print(".");
  }

  // Signal wifi is connected
  digitalWrite(0, LOW);

  Serial.println("");
  Serial.println("WiFi connected");

  server.on("/echo", handleEchoQueryArgumentsAsJSON);
  server.on("/settings", handleSettings);
  server.on("/sunrise", handleSunrise);
  server.on("/open", handleOpen);
  server.on("/stop", handleStop);

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loopWiFi() {
  server.handleClient();    //Handling of incoming requests
}

void jsonResponse(int code, const String &content) {
  Serial.println("response:");
  Serial.print(content);
  server.send(code, "application/json", content);
}

void handleEchoQueryArgumentsAsJSON() {
  Serial.println("handleEchoQueryArgumentsAsJSON");

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  for (int i = 0; i < server.args(); i++) {
    json[server.argName(i)] = server.arg(i);
  } 

  String jsonStr;
  json.prettyPrintTo(jsonStr);
  jsonResponse(200, jsonStr);
}

void handleSettings () {
  Serial.println("handleSettings");

  int newSteps = server.arg("steps").toInt();
  int newSpeed = server.arg("speed").toInt();
  int newAcceleration = server.arg("acceleration").toInt();

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["time"] = now();
  json["url"] = server.uri();
  json["steps"] = newSteps;
  json["speed"] = newSpeed;
  json["acceleration"] = newAcceleration;

  if(newSteps < 1 || newSpeed < 1 || newAcceleration < 1) {
    json["error"] = "Invalid request data";

    String jsonStr;
    json.prettyPrintTo(jsonStr);
    jsonResponse(400, jsonStr);
    return;
  }

  steps = newSteps;
  speed = newSpeed;
  acceleration = newAcceleration;

  String jsonStr;
  json.prettyPrintTo(jsonStr);
  jsonResponse(200, jsonStr);
}

void handleSunrise () {
  Serial.println("handleSunrise");

  int newIsEnabled = server.arg("isEnabled").toInt();
  float newAmount = server.arg("amount").toFloat();
  int newSunriseHour = server.arg("hour").toInt();
  int newSunriseMinutes = server.arg("minutes").toInt();  

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["time"] = now();
  json["url"] = server.uri();
  json["isEnabled"] = newIsEnabled;
  json["amount"] = newAmount;
  json["sunriseHour"] = newSunriseHour;
  json["sunriseMinutes"] = newSunriseMinutes;

  if(server.arg("secondsGtmOffset")) {
    String secondsGtmOffsetStr = server.arg("secondsGtmOffset");
    int newSecondsGtmOffset = secondsGtmOffsetStr.toInt();
    if(secondsGtmOffsetStr != "" && newSecondsGtmOffset != secondsGtmOffset) {
      Serial.println("Timezone changed, adjust time");
      Serial.println(newSecondsGtmOffset - secondsGtmOffset);
      adjustTime(newSecondsGtmOffset - secondsGtmOffset);
      secondsGtmOffset = newSecondsGtmOffset;
    }

    json["secondsGtmOffset"] = newSecondsGtmOffset;
  }

  if(
    newIsEnabled < 0 || newIsEnabled > 1 ||
    amount <= 0 || amount > 1 ||
    sunriseHour < 0 || sunriseHour > 23 || 
    sunriseMinutes < 0 || sunriseMinutes > 59
  ) {
    json["error"] = "Invalid request data";

    String jsonStr;
    json.prettyPrintTo(jsonStr);
    jsonResponse(400, jsonStr);
    return;
  }

  isEnabled = newIsEnabled == 1 ? true : false;
  amount = newAmount;
  sunriseHour = newSunriseHour;
  sunriseMinutes = newSunriseMinutes;

  String jsonStr;
  json.prettyPrintTo(jsonStr);
  jsonResponse(200, jsonStr);
}

void handleOpen () {
  Serial.println("handleOpen");

  float openAmount = server.arg("amount").toFloat();
  int stepsToRotate = openAmount * steps;

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["time"] = now();
  json["url"] = server.uri();
  json["steps"] = steps;
  json["speed"] = speed;
  json["acceleration"] = acceleration;
  json["openAmount"] = openAmount;
  json["stepsToRotate"] = stepsToRotate;

  if(openAmount <= 0 || openAmount > 1) {
    json["error"] = "Invalid request data";

    String jsonStr;
    json.prettyPrintTo(jsonStr);
    jsonResponse(400, jsonStr);
    return;
  }

  Serial.print("Open curtains: steps to rotate:");
  Serial.print(stepsToRotate);
  enableOutputs();
  setMaxSpeed(speed);
  setAcceleration(acceleration);
  setCurrentPosition(0);
  moveTo(stepsToRotate);

  String jsonStr;
  json.prettyPrintTo(jsonStr);
  jsonResponse(200, jsonStr);
}

void handleStop () {
  Serial.println("handleStop");

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["time"] = now();
  json["url"] = server.uri();
  json["stop"] = "true";

  String jsonStr;
  json.prettyPrintTo(jsonStr);
  jsonResponse(200, jsonStr);
  stop();
}








// ###################################################
// ################# Timeserver ######################
// ###################################################

time_t getNtpTime();

void setupTimeSync() {
  udp.begin(timeSyncUDPPort);
  setSyncProvider(getNtpTime); // will sync every 5 minutes
}

void sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

time_t getNtpTime()
{
  WiFi.hostByName(ntpServerName, timeServerIP); 
  sendNTPpacket(timeServerIP);
  delay(500);
  int cb = udp.parsePacket();
  if (!cb) {
    delay(1);
  } else {
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long secsSince1900;
    // convert four bytes starting at location 40 to a long integer
    secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
    secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
    secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
    secsSince1900 |= (unsigned long)packetBuffer[43];
    unsigned long epoch  = secsSince1900 - 2208988800UL + secondsGtmOffset;
    Serial.print("epoch: ");
    Serial.println(epoch);
    return epoch;
  }

  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

void printDigits(int digits){
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay(){
  Serial.print(year()); 
  Serial.print("-");
  Serial.print(month());
  Serial.print("-");
  Serial.print(day());  
  Serial.print("-");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(); 
}



// ###################################################
// ################# Main ############################
// ###################################################

void setup() {
  Serial.begin(115200);

  // LED
  pinMode(0, OUTPUT);

  setupMotor();
  setupWifi();
  setupTimeSync();
}

void loop() {
  loopMotor();
  loopWiFi();
}
