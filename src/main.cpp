#include <Arduino.h>
// https://github.com/luni64/TeensyStep
#include <StepControl.h>
// https://github.com/kroimon/Arduino-SerialCommand
#include <SerialCommand.h>

#define motorStepPin 2
#define motorDirPin 3
#define motorOnOffPin 4

#define defaultSpeed 6400
#define defaultStartSpeed 1600
#define defaultAcceleration 1600

Stepper motor(motorStepPin, motorDirPin);
StepControl<> controller;
SerialCommand sCmd;

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

void setNextParamAsStartSpeed() {
  int value = getParam();
  Serial.print("motor.setPullInSpeed: ");
  Serial.println(value);
  motor.setPullInSpeed(value);
}

void setNextParamAsAcceleration() {
  int value = getParam();
  Serial.print("motor.setAcceleration: ");
  Serial.println(value);
  motor.setAcceleration(value);
}

void setDefaultParams() {
  motor.setMaxSpeed(defaultSpeed);
  motor.setPullInSpeed(defaultStartSpeed);
  motor.setAcceleration(defaultAcceleration);
}

// ----------------- Helpers -----------------

void motorOn() {
  Serial.println("turn motor on");
  digitalWrite(motorOnOffPin, HIGH);
}

void motorOff() {
  Serial.println("turn motor off");
  digitalWrite(motorOnOffPin, LOW);
}

// ----------------- Commands -----------------

void unrecognizedCommand(const char *command) {
  Serial.print("- unrecognizedCommand: ");
  Serial.println(command);
}

void setSettings() {
  Serial.println("- setSettings");  
  setNextParamAsMaxSpeed();
  setNextParamAsStartSpeed();
  setNextParamAsAcceleration();
}

void startRotation() {
  Serial.println("- startRotation");
  motorOn();
  controller.rotateAsync(motor);
}

void stopRotation() {
  Serial.println("- stopRotation");
  controller.stop();
  motorOff();
}

void rotateSteps() {
  Serial.println("- rotateSteps");
  
  motorOn();

  int steps = getParam();
  Serial.print("motor.setTargetRel: ");
  Serial.println(steps);
  motor.setTargetRel(steps);
  controller.move(motor);

  motorOff();
}

// ----------------- Setup -----------------

void setup(){
  Serial.begin(9600);
  Serial.println("- Setup start");

  pinMode(motorStepPin, OUTPUT);

  sCmd.addCommand("SET_SETTINGS", setSettings);
  sCmd.addCommand("START_ROTATION", startRotation);
  sCmd.addCommand("STOP_ROTATION", stopRotation);
  sCmd.addCommand("ROTATE_X_STEPS", rotateSteps);
  sCmd.setDefaultHandler(unrecognizedCommand);

  motorOff();

  Serial.println("- Setup end");
}

void loop() {
  // check for new commands the whole time
  sCmd.readSerial();
}
