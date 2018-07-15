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

// https://github.com/bblanchon/ArduinoJson
#include <ArduinoJson.h>



// ###################################################
// ################# Motor code ######################
// ###################################################

// Pins on the Huzzah microcontroller
#define motorStepPin 4
#define motorDirPin 5
#define motorOnOffPin 16
#define switchPin 14

// Default settings
#define defaultSpeed 1000
#define defaultAcceleration 1000

SerialCommand sCmd;

AccelStepper motor(AccelStepper::DRIVER, motorStepPin, motorDirPin);
int switchValue = LOW;

// ---------------- params -----------------

int getSerialParam() {
  char *arg = sCmd.next();
  if (arg == NULL) {
    return 0;
  }

  return atoi(arg);
}

void setNextParamAsMaxSpeed() {
  int value = getSerialParam();
  Serial.print("motor.setMaxSpeed: ");
  Serial.println(value);
  motor.setMaxSpeed(value);
}

void setNextParamAsAcceleration() {
  int value = getSerialParam();
  Serial.print("motor.setAcceleration: ");
  Serial.println(value);
  motor.setAcceleration(value);
}

void setDefaultParams() {
  motor.setMaxSpeed(defaultSpeed);
  motor.setAcceleration(defaultAcceleration);
}

// ----------------- Helpers -----------------

void motorOn() {
  Serial.println("turn motor on");  
  motor.enableOutputs();
}

void motorOff() {
  Serial.println("turn motor off");  
  motor.disableOutputs();
}

// ----------------- Commands -----------------

void unrecognizedCommand(const char *command) {
  Serial.print("- unrecognizedCommand: ");
  Serial.println(command);
}

void setSpeed() {
  Serial.println("- setSpeed");  
  setNextParamAsMaxSpeed();
}

void setAcceleration() {
  Serial.println("- setAcceleration");  
  setNextParamAsAcceleration();
}

void stop() {
  Serial.println("- stop");
  Serial.println("motor.stop");

  // motor.moveTo(motor.currentPosition());
  motor.stop();
}

void rotateSteps() {
  Serial.println("- rotateSteps");

  motor.enableOutputs();
  motor.setCurrentPosition(0);

  int steps = getSerialParam();
  Serial.print("motor.moveTo: ");
  Serial.println(steps);
  motor.moveTo(steps);
}

void rotateFixedSteps() {
  Serial.println("- rotateFixedSteps");

  motor.enableOutputs();
  motor.setCurrentPosition(0);

  Serial.print("motor.moveTo: ");
  motor.moveTo(2000);
}

// ----------------- Loop Helpers -----------------


void printSwitchValue(int switchValue) {
    if (switchValue == HIGH) {
      Serial.println("switchPin == HIGH");
    }
  
    if (switchValue == LOW) {
      Serial.println("switchPin == LOW");
    }
}

void checkSwitch() {
  // int oldValue = switchValue;

  switchValue = digitalRead(switchPin);
  digitalWrite(0, switchValue);

  // if(oldValue != switchValue) {
  //   printSwitchValue(switchValue);
  // }

  Serial.print("pins: ");
  for (int i = 0; i < 20; i++) {
    Serial.print(" ");
    Serial.print(digitalRead(i));
  }
  Serial.println(" ");
  
  // printSwitchValue(switchValue);
}
// ----------------- Setup -----------------

void setupMotor(){
    Serial.println("- Setup start");

    pinMode(0, OUTPUT);
    pinMode(motorStepPin, OUTPUT);

    motor.setEnablePin(motorOnOffPin);
    motor.disableOutputs();
    setDefaultParams();

    sCmd.addCommand("STEPS", rotateSteps);

    sCmd.addCommand("MOTOR_ON", motorOn);
    sCmd.addCommand("MOTOR_OFF", motorOff);

    sCmd.addCommand("SPEED", setSpeed);
    sCmd.addCommand("ACCEL", setAcceleration);

    sCmd.addCommand("STOP", stop);

    Serial.println("- Setup end");
}

void loopMotor()
{    
    // Read commands
    sCmd.readSerial();

    // Auto turn motor off when it's done
    if (motor.distanceToGo() == 0) {
	    motor.disableOutputs();
    }

    if(hour() == sunriseHour && minute() == sunriseMinutes) {
      Serial.print("sunrise time");
      motor.enableOutputs();
      motor.setCurrentPosition(0);
      motor.moveTo(stepsToPerformLater);
    }

    motor.run();
}







// ###################################################
// ################# Wifi code ######################
// ###################################################

const char* ssid     = "H369AB21CAF";
const char* password = "9DA66C3A229E";

int steps = 45000;
int speed = 5000;
int acceleration = 5000;

bool isEnabled = false;
float amount = 0.75;
int sunriseHour = 8;
int sunriseMinutes = 0;

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

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
  int newSpeed = server.arg("steps").toInt();
  int newAcceleration = server.arg("steps").toInt();

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["url"] = server.uri();
  json["steps"] = steps;
  json["speed"] = speed;
  json["acceleration"] = acceleration;

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
  int currentTime = server.arg("currentTime").toInt();
  
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["url"] = server.uri();
  json["isEnabled"] = isEnabled;
  json["amount"] = amount;
  json["sunriseHour"] = sunriseHour;
  json["sunriseMinutes"] = sunriseMinutes;
  json["currentTime"] = currentTime;

  if(
    newIsEnabled < 0 || newIsEnabled > 1 ||
    amount <= 0 || amount > 1 ||
    sunriseHour < 0 || sunriseHour > 23 || 
    sunriseMinutes < 0 || sunriseMinutes > 59
    currentTime > 1531677842
  ) {
    json["error"] = "Invalid request data";

    String jsonStr;
    json.prettyPrintTo(jsonStr);
    jsonResponse(400, jsonStr);
    return;
  }

  isEnabled = newIsEnabled == 1 ? true : false;
  amount = newAmount;
  newSunriseHour = newSunriseHour;
  newSunriseMinutes = newSunriseMinutes;

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
  motor.enableOutputs();
  motor.setMaxSpeed(speed);
  motor.setAcceleration(acceleration);
  motor.setCurrentPosition(0);
  motor.moveTo(stepsToRotate);

  String jsonStr;
  json.prettyPrintTo(jsonStr);
  jsonResponse(200, jsonStr);
}

void handleStop () {
  Serial.println("handleStop");

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["url"] = server.uri();
  json["stop"] = "true";

  String jsonStr;
  json.prettyPrintTo(jsonStr);
  jsonResponse(200, jsonStr);
  stop();
}





// ###################################################
// ################# Main ############################
// ###################################################

void setup() {
  Serial.begin(115200);
  sCmd.setDefaultHandler(unrecognizedCommand);

  // LED
  pinMode(0, OUTPUT);

  setupMotor();
  setupWifi();
}

void loop() {
  loopMotor();
  loopWiFi();
}
