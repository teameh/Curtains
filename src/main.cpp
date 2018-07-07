#include <Arduino.h>
// http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <AccelStepper.h>
// https://github.com/kroimon/Arduino-SerialCommand
#include <SerialCommand.h>

// Pins on the Huzzah microcontroller
#define ledPin 0
#define motorStepPin 4
#define motorDirPin 5
#define motorOnOffPin 16
#define switchPin 14

// Default settings
#define defaultSpeed 1000
#define defaultAcceleration 1000

SerialCommand sCmd;

// ################# Motor code #################

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
  Serial.println("- setSettings");  
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

// ################# Main #################

void setup() {
  Serial.begin(9600);
  sCmd.setDefaultHandler(unrecognizedCommand);

  setupMotor();
}

void loop() {
  loopMotor();
}