#include <Arduino.h>

// http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <AccelStepper.h>

// https://github.com/kroimon/Arduino-SerialCommand
#include <SerialCommand.h>

// https://github.com/esp8266/Arduino
#include <ESP8266WiFi.h>










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
int loopCounter = 0;
int switchValue = LOW;

// ---------------- params -----------------

int getParam() {
  char *arg = sCmd.next();
  if (arg == NULL) {
    return 0;
  }

  return atoi(arg);
}

void setNextParamAsMaxSpeed() {
  int value = getParam();
  Serial.print("motor.setMaxSpeed: ");
  Serial.println(value);
  motor.setMaxSpeed(value);
}

void setNextParamAsAcceleration() {
  int value = getParam();
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

  int steps = getParam();
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

    motor.run();
    
    // checkSwitch();
}











// ###################################################
// ################# Wifi code ######################
// ###################################################

const char* ssid     = "H369AB21CAF";
const char* password = "9DA66C3A229E";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

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
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loopWiFi() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  String reqPrefix = "/steps/";
  int prefixIndex = req.indexOf(reqPrefix);

  // Match the request
  if (prefixIndex == -1) {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  int slashIndex0 = req.indexOf('/', prefixIndex + reqPrefix.length());
  String steps = "";
  if(slashIndex0 != -1) {
    steps = req.substring(prefixIndex + reqPrefix.length(), slashIndex0);
    motor.enableOutputs();
    motor.setCurrentPosition(0);
    motor.moveTo(steps.toInt());
  }

  int slashIndex1 = req.indexOf('/', slashIndex0+1);
  String speed = "";
  if(slashIndex1 != -1) {
    speed = req.substring(slashIndex0+1, slashIndex1);
    motor.setMaxSpeed(speed.toInt());
  }

  int slashIndex2 = req.indexOf('/', slashIndex1+1);
  String acceleration = "";
  if(slashIndex2 != -1) {
    acceleration = req.substring(slashIndex1+1, slashIndex2);
    motor.setAcceleration(acceleration.toInt());
  }

  client.flush();

  // Prepare the response
  String nl = "\r\n";
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML><html>" + nl;
  s += nl + "<h1>Rotate!</h1>" ;
  s += nl + "<pre><code>";
  s += nl + "req: " + req + " ";
  s += nl;
  s += nl + "slashIndex0: " + slashIndex0 + " ";
  s += nl + "slashIndex1: " + slashIndex1 + " ";
  s += nl + "slashIndex2: " + slashIndex2 + " ";
  s += nl;
  s += nl + "steps: " + steps + " ";
  s += nl + "speed: " + speed + " ";
  s += nl + "acceleration: " + acceleration + " ";
  s += nl + "</pre></code>";
  s += nl + "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}






// ###################################################
// ################# Main ############################
// ###################################################

void setup() {
  Serial.begin(115200);
  sCmd.setDefaultHandler(unrecognizedCommand);

  setupMotor();
  setupWifi();
}

void loop() {
  loopMotor();
  loopWiFi();
}