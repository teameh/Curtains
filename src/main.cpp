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
#include <ESP8266HTTPClient.h>

#include <WiFiUdp.h>

// https://github.com/bblanchon/ArduinoJson
#include <ArduinoJson.h>

// https://github.com/firebase/firebase-arduino
#include <FirebaseArduino.h>

// ###################################################
// ################# INIT ############################
// ###################################################

// ----------------- WIFI ----------------------------

#define WIFI_SSID "H369AB21CAF"
#define WIFI_PASSWORD "9DA66C3A229E"

int steps = 45000;
int speed = 5000;
int acceleration = 5000;

bool isEnabled = false;
float amount = 0.25;
int sunriseHour = 8;
int sunriseMinutes = 0;
bool sunHasJustRisen = false;
bool prevIsRunning = false;

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

// ----------------- Time ----------------------------

#define LOCAL_UDP_PORT 2390 // local port to listen for UDP packets
#define NTP_PACKET_SIZE 48
IPAddress timeServerIP;
const char* ntpServerName = "time.nist.gov";
byte packetBuffer[NTP_PACKET_SIZE];
WiFiUDP udp;
WiFiUDP Udp;

int secondsGtmOffset = 7200; // GTM+1
int prevMinute = 0;
int prevSecond = 0;
int timeSyncFailCount = 0;
int timeSyncInterval = 10; // start with every ten seconds until we're synced.

void printDateTime();

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

void setMotorDefaults() {
  Serial.println("- setMotorDefaults");
  setMaxSpeed(speed);
  setAcceleration(acceleration);
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
  Serial.println("- startSunrise");
  stop();
  enableOutputs();
  setMaxSpeed(speed);
  setAcceleration(acceleration);
  setCurrentPosition(0);
  moveTo(amount * steps);
}

void checkSunRise() {
  if(isEnabled && hour() == sunriseHour && minute() == sunriseMinutes) {
    startSunrise();
  }
}

// ----------------- Setup -----------------

void setupMotor(){
    Serial.println("- setupMotor");

    pinMode(motorStepPin, OUTPUT);

    motor.setEnablePin(motorOnOffPin);
    disableOutputs();
    setMotorDefaults();
    Serial.flush();
}

void loopMotor()
{
  bool isRunning = motor.run();
  if(!isRunning && prevIsRunning) {
    // Auto turn motor off when it's done rotating
    disableOutputs();
  }
  prevIsRunning = isRunning;
}


// ###################################################
// ################# Firebase   ######################
// ###################################################

// Set these to run example.
#define FIREBASE_HOST "sunrise-9a00b.firebaseio.com"
#define FIREBASE_AUTH "EizuhQ67ZmCuqIrt9FUeXniag5kVXPwAVhp6pxM5"

int getFirebaseInt(const String& path);
float getFirebaseFloat(const String& path);
bool getFirebaseBool(const String& path);
bool setFirebaseIntFloatBool(const String& path, const JsonVariant& value);
void getPrevValuesFromFirebase();

bool isStreaming = false;

void setupFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  getPrevValuesFromFirebase();
}

void getPrevValuesFromFirebase() {
  int newSteps = getFirebaseInt("steps");
  if (!Firebase.failed() && newSteps > 0) {
    steps = newSteps;
  }
  int newSpeed = getFirebaseInt("speed");
  if (!Firebase.failed() && newSpeed > 0) {
    speed = newSpeed;
  }
  int newAcceleration = getFirebaseInt("acceleration");
  if (!Firebase.failed() && newAcceleration > 0){
    acceleration = newAcceleration;
  }
  int newSunriseHour = getFirebaseInt("sunriseHour");
  if (!Firebase.failed()) {
    sunriseHour = newSunriseHour;
  }
  int newSunriseMinutes = getFirebaseInt("sunriseMinutes");
  if (!Firebase.failed()) {
    sunriseMinutes = newSunriseMinutes;
  }

  bool newIsEnabled = getFirebaseBool("isEnabled");
  if (!Firebase.failed()) {
    isEnabled = newIsEnabled;
  }
  
  float newAmount = getFirebaseFloat("amount");
  if (!Firebase.failed() && newAmount > 0){
    amount = newAmount;
  }
}

int getFirebaseInt(const String& path) {
  Serial.print("- getFirebaseInt: ");
  Serial.println(path);

  int value = Firebase.getInt(path);
  if (Firebase.failed()) {
    Serial.println("Firebase get failed");
    Serial.println(Firebase.error());
  } else {
    Serial.print("value: ");
    Serial.println(value);
  }
  delay(1);
  return value;
}

float getFirebaseFloat(const String& path) {
  Serial.print("- getFirebaseInt: ");
  Serial.println(path);

  float value = Firebase.getFloat(path);
  if (Firebase.failed()) {
    Serial.println("Firebase get failed");
    Serial.println(Firebase.error());
  } else {
    Serial.print("value: ");
    Serial.println(value);
  }
  delay(1);
  return value;
}

bool getFirebaseBool(const String& path) {
  Serial.print("- getFirebaseInt: ");
  Serial.println(path);

  bool value = Firebase.getBool(path);
  if (Firebase.failed()) {
    Serial.println("Firebase get failed");
    Serial.println(Firebase.error());
  } else {
    Serial.print("value: ");
    Serial.println(value);
  }
  delay(1);
  return value;
}

bool setFirebaseIntFloatBool(const String& path, const JsonVariant& value) {
  Serial.print("- setFirebaseValue ");
  Serial.println(path);

  Firebase.set(path, value);
  if (Firebase.failed()) {
      Serial.print("Failed: ");
      Serial.println(Firebase.error());  
      return false;
  }
  delay(1);
  return true;
}

// ###################################################
// ################# Wifi code ######################
// ###################################################



void handleEchoQueryArgumentsAsJSON();
void handleSettings ();
void handleSunrise ();
void handleOpen ();
void handleStop ();

bool wifiBlink = HIGH;

void setupWifi() {
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.println("- setupWifi");
  Serial.printf("Connecting to %s.", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    // blinking wifi is not connected
    digitalWrite(0, wifiBlink);
    if(wifiBlink) {
      wifiBlink = wifiBlink == HIGH ? LOW : HIGH;
    }
    Serial.print(".");
  }

  Serial.println("connected!");

  // Signal wifi is connected
  digitalWrite(0, HIGH);

  server.on("/echo", handleEchoQueryArgumentsAsJSON);
  server.on("/settings", handleSettings);
  server.on("/setSunrise", handleSunrise);
  server.on("/open", handleOpen);
  server.on("/stop", handleStop);

  // Start the server
  server.begin();
  Serial.print("Webserver started, ip: http://");
  Serial.println(WiFi.localIP());
  Serial.flush();
}

void loopWiFi() {
  server.handleClient();    //Handling of incoming requests
}

void jsonResponse(int code, const String &content) {
  Serial.println("jsonResponse:");
  Serial.println(content);
  server.send(code, "application/json", content);
}

void handleEchoQueryArgumentsAsJSON() {
  Serial.println("- handleEchoQueryArgumentsAsJSON");

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
  Serial.println("- handleSettings");

  int newSteps = server.arg("steps").toInt();
  int newSpeed = server.arg("speed").toInt();
  int newAcceleration = server.arg("acceleration").toInt();

  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["time"] = now();
  json["url"] = server.uri();
  json["steps"] = newSteps;
  json["speed"] = newSpeed;
  json["acceleration"] = newAcceleration;

  if(newSteps < 1 || newSpeed < 1 || newAcceleration < 1) {
    Serial.println("Invalid request data");
    
    if (newSteps < 1) {
      Serial.print("newSteps: ");
      Serial.print(newSteps);
      Serial.print(" is invalid");
    }

    if (newSpeed < 1) {
      Serial.print("newSpeed: ");
      Serial.print(newSpeed);
      Serial.print(" is invalid");
    }

    if (newAcceleration < 1) {
      Serial.print("newAcceleration: ");
      Serial.print(newAcceleration);
      Serial.print(" is invalid");
    }

    json["error"] = "Invalid request data";

    String jsonStr;
    json.prettyPrintTo(jsonStr);
    jsonResponse(400, jsonStr);
    return;
  }

  steps = newSteps;
  speed = newSpeed;
  acceleration = newAcceleration;

  bool success = setFirebaseIntFloatBool("steps", steps);
  success &= setFirebaseIntFloatBool("speed", speed);
  success &= setFirebaseIntFloatBool("acceleration", acceleration);

  if(!success) {
    json["error"] = "Firebase request failed";
    String jsonStr;
    json.prettyPrintTo(jsonStr);
    jsonResponse(500, jsonStr);
    return;
  }

  String jsonStr;
  json.prettyPrintTo(jsonStr);
  jsonResponse(200, jsonStr);
}

void handleSunrise () {
  Serial.println("- handleSunrise");

  int newIsEnabled = server.arg("isEnabled").toInt();
  float newAmount = server.arg("amount").toFloat();
  int newSunriseHour = server.arg("hour").toInt();
  int newSunriseMinutes = server.arg("minutes").toInt();

  StaticJsonBuffer<512> jsonBuffer;
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
    newAmount <= 0 || newAmount > 1 ||
    newSunriseHour < 0 || newSunriseHour > 23 ||
    newSunriseMinutes < 0 || newSunriseMinutes > 59
  ) {
    Serial.println("Invalid request data");
    
    if (newIsEnabled < 0 || newIsEnabled > 1) {
      Serial.print("newIsEnabled: ");
      Serial.print(newIsEnabled);
      Serial.print(" is invalid");
    }

    if (newAmount <= 0 || newAmount > 1) {
      Serial.print("newAmount: ");
      Serial.print(newAmount);
      Serial.print(" is invalid");
    }

    if (newSunriseHour < 0 || newSunriseHour > 23) {
      Serial.print("newSunriseHour: ");
      Serial.print(newSunriseHour);
      Serial.print(" is invalid");
    }

    if (newSunriseMinutes < 0 || newSunriseMinutes > 5) {
      Serial.print("newSunriseMinutes: ");
      Serial.print(newSunriseMinutes);
      Serial.print(" is invalid");
    }

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

  bool success = setFirebaseIntFloatBool("isEnabled", isEnabled);
  success &= setFirebaseIntFloatBool("amount", amount);
  success &= setFirebaseIntFloatBool("sunriseHour", sunriseHour);
  success &= setFirebaseIntFloatBool("sunriseMinutes", sunriseMinutes);

  if(!success) {
    json["error"] = "Firebase request failed";
    String jsonStr;
    json.prettyPrintTo(jsonStr);
    jsonResponse(500, jsonStr);
    return;
  }

  String jsonStr;
  json.prettyPrintTo(jsonStr);
  jsonResponse(200, jsonStr);
}

void handleOpen () {
  Serial.println("- handleOpen");

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
  Serial.println("- handleStop");

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
  udp.begin(LOCAL_UDP_PORT);
  setSyncInterval(10);
  setSyncProvider(getNtpTime);
  Serial.flush();
}

void sendNTPpacket(IPAddress& address)
{
  Serial.println("- sendNTPpacket");
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
  Serial.println("- getNtpTime");
  
  if(!timeServerIP) {
    Serial.print("get timeServerIP by hostname lookup...: ");
    WiFi.hostByName(ntpServerName, timeServerIP);
    Serial.println(timeServerIP);
  } else {
    Serial.print("reuse timeServerIP: ");
    Serial.println(timeServerIP);
  }

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
    Serial.print("UNIX epoch: ");
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

void printDateTime(){
  Serial.print(year());
  Serial.print("-");
  Serial.print(month());
  Serial.print("-");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void decreaseTimeSyncInterval() {
  Serial.println("- decreaseTimeSyncInterval");
  timeSyncInterval = 15 * 60;
  setSyncInterval(timeSyncInterval);
  // indicate time is synced
  digitalWrite(0, LOW);
}


void onceEverySecond() {
  printDateTime();
  // Serial.print("- motor.speed: ");
  // Serial.println(motor.speed());

  bool timeIsSynced = now() > 1514764800; // 2018
  // Decrease sync interval if time is synced or we've tried too many times
  if(timeSyncInterval == 10 && (timeIsSynced || timeSyncFailCount > 10)) {
    decreaseTimeSyncInterval();
  }
}

void loopTimeSync() {
  if (minute() != prevMinute) {
    prevMinute = minute();
    checkSunRise();
  }

  if (second() != prevSecond) {
    prevSecond = second();
    onceEverySecond();
  }
}


// ###################################################
// ################# Main ############################
// ###################################################

void setup() {
  Serial.begin(115200);

  delay(2000);

  // LED
  pinMode(0, OUTPUT);

  setupMotor();
  setupWifi();
  setupTimeSync();
  setupFirebase();
}

void loop() {
  loopMotor();
  loopWiFi();
  loopTimeSync();
}
